#include "../include/node.h"
#include "../include/interpretator.h"
#include "../include/object.h"
#include "../include/symbol-table.h"
#include "../include/lexer.h"
#include "../include/parser.h"

#include "../include/utils.h"

#include "../include/repl/readfile.h"
#include "../include/repl/repl.h"

#include "../include/memarena.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef ErrType (*BinOpFn)(Number* dest, const Number* src);

static BinOpFn binOpdispatch[] = {
  [TOK_PLUS]  = addNumber,
  [TOK_MINUS] = subNumber,
  [TOK_MUL] = mulNumber,
  [TOK_DIV] = divNumber,
  [TOK_POW] = powNumber,

  [TOK_EE] = isEqualNumber,
  [TOK_LT] = isLessThanNumber,
  [TOK_GT] = isGreaterThanNumber,
  [TOK_LTE] = isLessThanEqualNumber,
  [TOK_GTE] = isGreaterThanEqualNumber,
  [TOK_NE] = isNotEqualNumber,

  [TOK_AND] = andNumber,
  [TOK_OR] = orNumber,
};

Interpretator* initInterpretator(char *filename, char *sourcetext, Error **err, SymbolTable* variables) {
  if (!filename || !sourcetext || !err || !variables) return NULL;

  Interpretator* interpretator = arenaAlloc(parseArena, sizeof(Interpretator));

  if (!interpretator) return NULL;

  interpretator->filename = filename;
  interpretator->sourcetext = sourcetext;
  interpretator->err = err;
  interpretator->variables = variables;

  return interpretator;
}

Object* visitNumberNode(ASTNode* node, Interpretator* ctx) {
  (void)ctx;

  NumberNode* num = (NumberNode*)node;

  if (num->token.type == TOK_FLOAT) {
    return (Object*)initFloat(num->token.val.f);
  }

  return (Object*)initInt(num->token.val.i);
}

static inline void _initBinOpTypeErr(Position start, Position end, const char* op, const char *leftType, const char* rightType, char *filename, char *sourcetext, Error **err) {
  char buffer[256];

  snprintf(buffer, sizeof(buffer), "Unsupported operand types for '%s': '%s' %s '%s'", op, leftType, op, rightType);

  if (!*err) *err = initTypeError(start, end, filename, buffer, sourcetext);
}

static Object* _stringBinOp(BinOpNode* binOper, Object* srcObj, const char* op, Object* destObj, const char *leftType, const char *rightType, char *filename, char *sourcetext, Error **err) {
  Object* out = NULL;
  
  switch (binOper->operTok.type) {
    case TOK_PLUS:
      if (srcObj->type != OBJ_STRING || destObj->type != OBJ_STRING) {
        _initBinOpTypeErr(binOper->operTok.start, binOper->operTok.end, op, leftType, rightType, filename, sourcetext, err);

        freeObject(srcObj);
        freeObject(destObj);

        return NULL;
      }

      out = (Object*)addString((String*)srcObj, (String*)destObj);
      break;

    case TOK_MUL:
      if (srcObj->type != OBJ_STRING || destObj->type != OBJ_NUMBER_INT) {
        _initBinOpTypeErr(binOper->operTok.start, binOper->operTok.end, op, leftType, rightType, filename, sourcetext, err);

        freeObject(srcObj);
        freeObject(destObj);

        return NULL;
      }

      out = (Object*)mulString((String*)srcObj, (Number*)destObj);
      break;

    case TOK_EE: {
      Number* res = NULL;

      if ((srcObj->type == OBJ_NUMBER_INT || srcObj->type == OBJ_NUMBER_FLOAT) || (destObj->type == OBJ_NUMBER_FLOAT || destObj->type == OBJ_NUMBER_INT)) {
        res = initInt(0);
      } else {
        String *lhs = (String*)srcObj;
        String *rhs = (String*)destObj;

        res = initInt(strcmp(lhs->value, rhs->value) == 0);
      }

      freeObject(srcObj);
      freeObject(destObj);

      return (Object*)res;
    }

    case TOK_NE: {
      Number* res = NULL;

      if ((srcObj->type == OBJ_NUMBER_INT || srcObj->type == OBJ_NUMBER_FLOAT) || (destObj->type == OBJ_NUMBER_FLOAT || destObj->type == OBJ_NUMBER_INT)) {
        res = initInt(0);
      } else {
        String *lhs = (String*)srcObj;
        String *rhs = (String*)destObj;

        res = initInt(strcmp(lhs->value, rhs->value) != 0);
      }

      freeObject(srcObj);
      freeObject(destObj);

      return (Object*)res; 
    }

    default:
      _initBinOpTypeErr(binOper->operTok.start, binOper->operTok.end, op, leftType, rightType, filename, sourcetext, err);
      freeObject(srcObj);
      freeObject(destObj);

      return NULL;
  }

  freeObject(srcObj);
  freeObject(destObj);

  return out;
}

Object* visitBinOpNode(ASTNode* node, Interpretator* ctx) {
  BinOpNode* binOper = (BinOpNode*)node;

  TokType opType = binOper->operTok.type;

  const char* op = binOpStr[opType];
  if (!op) op = "?";

  ASTNode* leftNode = binOper->leftNode;
  ASTNode* rightNode = binOper->rightNode;

  Object *srcObj = leftNode->visit(leftNode, ctx);
  Object *destObj = rightNode->visit(rightNode, ctx);

  if (!srcObj || !destObj) {
    freeObject(srcObj);
    freeObject(destObj);
    return NULL;
  }

  const char* leftType = typeofobj(srcObj);
  const char* rightType = typeofobj(destObj);
  
  ObjType srcType = srcObj->type;
  ObjType destType = destObj->type;

  bool isSrcNum = ((srcType == OBJ_NUMBER_INT) || (srcType == OBJ_NUMBER_FLOAT));
  bool isDestNum = ((destType == OBJ_NUMBER_INT) || (destType == OBJ_NUMBER_FLOAT));

  if (srcType == OBJ_STRING || destType == OBJ_STRING)
    return _stringBinOp(binOper, srcObj , op, destObj, leftType, rightType, ctx->filename, ctx->sourcetext, ctx->err);

  if (!isSrcNum || !isDestNum) {
    _initBinOpTypeErr(binOper->operTok.start, binOper->operTok.end, op, leftType, rightType, ctx->filename, ctx->sourcetext, ctx->err);

    freeObject(srcObj);
    freeObject(destObj);

    return NULL;
  }

  Number* src = (Number*)srcObj;
  Number* dest = (Number*)destObj;

  if (!src || !dest) {
    if (!*ctx->err) *ctx->err = initValueError(binOper->operTok.start, binOper->operTok.end, ctx->filename, "Expected TOK_FLOAT or TOK_INT token, received NULL", ctx->sourcetext);

    freeObject(srcObj);
    freeObject(destObj);

    return NULL;
  }

  if (dest->isStatic) {
    dest = copyNumber(dest);

    if (!dest) {
      freeObject(srcObj);
      return NULL;
    }
  }

  ErrType output;

  BinOpFn fn = binOpdispatch[opType];

  if (fn) {
    output = fn(dest, src);
  } else {
    output = -1;
  }

  switch (output) {
    case ERR_NONE:
      break;
    case ERR_NULL:
      if (!*ctx->err) *ctx->err = initValueError(binOper->operTok.start, binOper->operTok.end, ctx->filename, "Expected TOK_FLOAT or TOK_INT token, received NULL", ctx->sourcetext);
      break;
    case ERR_DIV_BY_ZERO:
      if (!*ctx->err) *ctx->err = initValueError(binOper->operTok.start, binOper->operTok.end, ctx->filename, "Division by zero", ctx->sourcetext);
      break;
    case ERR_TYPE:
      if (!*ctx->err) *ctx->err = initTypeError(binOper->operTok.start, binOper->operTok.end, ctx->filename, "Incompatible type for binary operation", ctx->sourcetext);
      break;
    default:
      if (!*ctx->err) *ctx->err = initValueError(binOper->operTok.start, binOper->operTok.end, ctx->filename, "Unknown error.", ctx->sourcetext);
      break;
  }

  freeObject((Object*)src);

  if (output != ERR_NONE) {
    freeObject((Object*)dest);
    return NULL;
  }

  return (Object*)dest;
}

Object* visitUnaryOpNode(ASTNode* node, Interpretator* ctx) {
  UnaryOpNode* unaryOper = (UnaryOpNode*)node;

  char* op;

  switch (unaryOper->operTok.type) {
    case TOK_PLUS: op = "+"; break;
    case TOK_MINUS: op = "-"; break;
    case TOK_NOT: op = "NOT"; break;
    default: op = "?"; break;
  }

  Object* numberObj = unaryOper->node->visit(unaryOper->node, ctx);

  if (!numberObj) {
    if (!*ctx->err) *ctx->err = initValueError(unaryOper->operTok.start, unaryOper->operTok.end, ctx->filename, "Expected Number* result, received NULL.", ctx->sourcetext);
    return NULL;
  }

  const char* type = typeofobj(numberObj);

  if (numberObj->type != OBJ_NUMBER_INT && numberObj->type != OBJ_NUMBER_FLOAT) {
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "Can't apply unary operation '%s' to %s", op, type);

    freeObject(numberObj);

    if (!*ctx->err) *ctx->err = initTypeError(unaryOper->operTok.start, unaryOper->operTok.end, ctx->filename, buffer, ctx->sourcetext);
    return NULL;
  }

  Number* num = (Number*)numberObj;

  switch (op[0]) {
    case '-': {
      Object* res;

      if (numberObj->type == OBJ_NUMBER_INT) {
        res = (Object*)initInt(-num->as.i);
      } else {
        res = (Object*)initFloat(-num->as.f);
      }

      freeObject(numberObj);
      return res;
    }

    case '+': {
      Object* res = copyObject(numberObj);
      freeObject(numberObj);
      return res;
    }

    case 'N': {
      Object *res;

      if (numberObj->type == OBJ_NUMBER_INT) {
        res = (Object*)initInt(!num->as.i);
      } else {
        res = (Object*)initFloat(!num->as.f);
      }

      freeObject(numberObj);
      return res;
    }

    default: 
      if (!*ctx->err) *ctx->err = initSyntaxError(unaryOper->operTok.start, unaryOper->operTok.end, ctx->filename, "Unknown unary operator.", ctx->sourcetext);
      return NULL;
  }

  return NULL;
}

Object* visitVarAccessNode(ASTNode* node, Interpretator* ctx) {
  VarAccessNode* va = (VarAccessNode*)node;
  char *varName = va->token.val.s;

  Object* stored = getTable(ctx->variables, varName);

  if (!stored) {
    int len = snprintf(NULL, 0, "Undefined variable \"%s\"", varName);
    char *buffer = malloc(len + 1);

    snprintf(buffer, len + 1, "Undefined variable \"%s\"", varName);

    if (!*ctx->err) *ctx->err = initNameError(va->token.start, va->token.end, ctx->filename, buffer, ctx->sourcetext);
    free(buffer);

    return NULL;
  }

  return copyObject(stored);
}

Object* visitVarAssignNode(ASTNode* node, Interpretator* ctx) {
  VarAssignNode* va = (VarAssignNode*)node;

  Object* value = va->value->visit(va->value, ctx);

  if (!value) return NULL;

  setTable(ctx->variables, va->identifier, value, true);

  return value;
}

Object* visitStringNode(ASTNode* node, Interpretator* ctx) {
  (void)ctx;

  StringNode* str = (StringNode*)node;

  return (Object*)initString(str->token.val.s, str->len);
}

Object* visitContinueNode(ASTNode* node, Interpretator* ctx) {
  (void)node;
  (void)ctx;

  return (Object*)initContinue();
}

Object* visitBreakNode(ASTNode* node, Interpretator* ctx) {
  (void)node;
  (void)ctx;

  return (Object*)initBreak();
}

Object* visitProgramNode(ASTNode* node, Interpretator* ctx) {
  ProgramNode* prog = (ProgramNode*)node;

  Object* last = NULL;

  for (size_t i = 0; i < prog->count; i++) {
    freeObject(last);
    last = prog->statements[i]->visit(prog->statements[i], ctx);

    if (!last) return NULL; // error already set
    
    if (last->type == OBJ_RETURN || last->type == OBJ_BREAK || last->type == OBJ_CONTINUE) {
      return last;
    }
  }

  return last;
}

Object* visitIfNode(ASTNode* n, Interpretator* ctx) {
  if (!n) return NULL;

  IfNode* node = (IfNode*)n;

  Object* condition = node->condition->visit(node->condition, ctx);

  if (!condition || condition->type != OBJ_NUMBER_INT) return NULL;

  Number* cond = (Number*)condition;

  if (cond->as.i != 0) {
    freeObject(condition);
    return node->thenExpr->visit(node->thenExpr, ctx);
  }

  freeObject(condition);

  for (size_t i = 0; i < node->elifCount; i++) {
    Object* elifVal = node->elifConds[i]->visit(node->elifConds[i], ctx);
    if (!elifVal) return NULL;

    if (elifVal->type != OBJ_NUMBER_INT) return NULL;

    if (((Number*)elifVal)->as.i != 0) {
      freeObject(elifVal);
      Object* obj = node->elifExprs[i]->visit(node->elifExprs[i], ctx);

      return obj;
    }

    freeObject(elifVal);
  }

  // Fall through to ELSE if present
  if (node->elseExpr) {
    Object* obj = node->elseExpr->visit(node->elseExpr, ctx);

    return obj;
  }

  return (Object*)initInt(0);
}

Object* visitListNode(ASTNode* node, Interpretator* ctx) {
  ListNode* listNode = (ListNode*)node;
  
  Object *objs[listNode->size]; // TODO: replace with heap allocation

  for (uint64_t i = 0; i < listNode->size; i++) {
    objs[i] = listNode->objects[i]->visit(listNode->objects[i], ctx);
  }

  List* list = initList(objs, listNode->size, listNode->capacity);

  return (Object*)list;
}

Object* visitIndexNode(ASTNode* node, Interpretator* ctx) {
  IndexNode* idx = (IndexNode*)node;

  const char* targetType; 
  const char* idxType; 

  Object* target = idx->target->visit(idx->target, ctx);

  if (!target) {
    return NULL;
  }

  targetType = typeofobj(target);

  Object* index = idx->index->visit(idx->index, ctx);

  if (!index) {
    return NULL;
  }

  idxType = typeofobj(index);

  if (target->type == OBJ_NUMBER_INT || target->type == OBJ_NUMBER_FLOAT) {
    char buff[256];

    snprintf(buff, sizeof(buff), "Object of type \"%s\" is not subscriptable.", targetType);

    if (!*ctx->err) *ctx->err = initTypeError(idx->start, idx->end, ctx->filename, buff, ctx->sourcetext);

    freeObject(target);
    freeObject(index);

    return NULL;
  }

  if (index->type != OBJ_NUMBER_INT) {
    char buff[256];

    snprintf(buff, sizeof(buff), "Cannot use a value of type \"%s\" as an index.", idxType);

    if (!*ctx->err) *ctx->err = initTypeError(idx->start, idx->end, ctx->filename, buff, ctx->sourcetext);

    freeObject(target);
    freeObject(index);

    return NULL;
  } 

  if (target->type == OBJ_STRING) {
    String* str = (String*)target;
    Number* id = (Number*)index;

    if (id->as.i < 0 || (uint64_t)id->as.i >= str->len) {
      if (!*ctx->err) *ctx->err = initIndexError(idx->start, idx->end, ctx->filename, "Index out of range.", ctx->sourcetext);

      freeObject(target);
      freeObject(index);

      return NULL;
    }
    
    char buff[2] = {str->value[id->as.i], '\0'};

    freeObject(target);
    freeObject(index);

    return (Object*)initString(buff, 1);
  }
  
  List* list = (List*)target;
  Number* id = (Number*)index;
  
  if (id->as.i < 0 || (uint64_t)id->as.i >= list->size) {
    if (*ctx->err == NULL) *ctx->err = initIndexError(idx->start, idx->end, ctx->filename, "Index out of range.", ctx->sourcetext);

    freeObject(target);
    freeObject(index);

    return NULL;
  } 

  Object* obj = copyObject(list->objects[id->as.i]);

  freeObject(target);
  freeObject(index);

  return obj;
}

Object* visitWhileNode(ASTNode* node, Interpretator* ctx) {
  WhileNode* whileNode = (WhileNode*)node;

  ASTNode* condNode = whileNode->condition;
  ASTNode* bodyNode = whileNode->body;

  while (true) {  
    Object* cond = condNode->visit(condNode, ctx);

    if (!cond) return NULL;

    if (cond->type != OBJ_NUMBER_INT) {
      if (*ctx->err == NULL) *ctx->err = initTypeError(whileNode->start, whileNode->end, ctx->filename, "Condition must be evaluated to integer type object.", ctx->sourcetext);

      freeObject(cond);

      return NULL;
    }

    int value = ((Number*)cond)->as.i;
    freeObject(cond);

    if (value == 0) break;

    Object* tmp = bodyNode->visit(bodyNode, ctx);
    if (!tmp) return NULL;
    
    if (tmp->type == OBJ_BREAK) {
      freeObject(tmp);
      break;
    }

    if (tmp->type == OBJ_CONTINUE) {
      freeObject(tmp);
      continue;
    }

    if (tmp && tmp->type == OBJ_RETURN) {
      return tmp;
    }

    freeObject(tmp);
  }

  return (Object*)initInt(1);
}

Object* visitFunctionNode(ASTNode* node, Interpretator* ctx) {
  (void)ctx;

  FunctionNode* func = (FunctionNode*)node;
  Object* funcObj = (Object*)initFunction(func);

  if (!funcObj) {
    return NULL;
  }

  setTable(ctx->variables, func->name, funcObj, false);

  return (Object*)initInt(1); // true
}

static Object** evaluateArgs(ASTNode** args, size_t argCount, Interpretator* ctx) {
  Object** evaluatedArgs = malloc(sizeof(Object*) * argCount);
  if (!evaluatedArgs) return NULL;

  for (size_t i = 0; i < argCount; i++) {
    evaluatedArgs[i] = args[i]->visit(args[i], ctx);

    if (!evaluatedArgs[i]) {
      for (size_t j = 0; j < i; j++) 
        freeObject(evaluatedArgs[j]);

      free(evaluatedArgs);
      return NULL;
    }
  }

  return evaluatedArgs;
}

Object* visitFunctionCallNode(ASTNode* node, Interpretator* ctx) {
  FunctionCallNode* fncallnode = (FunctionCallNode*)node;
  Object* calleeObj = fncallnode->callee->visit(fncallnode->callee, ctx);

  if (!calleeObj) return NULL;

  Object* res = NULL;

  if (calleeObj->type == OBJ_FUNCTION) {
    FunctionCall* call = initFunctionCall(fncallnode, calleeObj, ctx);

    if (!call) {
      freeObject(calleeObj);
      return NULL;
    }

    Function* func = (Function*)call->function;

    if (call->argCount != func->paramCount) {
      char buf[256];

      snprintf(buf, sizeof(buf), "Function '%s' expected %zu arguments, got %zu.", func->name, func->paramCount, call->argCount);
      if (!*ctx->err) *ctx->err = initRuntimeError(fncallnode->start, fncallnode->end, ctx->filename, buf, ctx->sourcetext);

      freeObject((Object*)call);
      freeObject(calleeObj);
      return NULL;
    }

    SymbolTable* prevTable = ctx->variables;
    ctx->variables = call->env;

    res = call->function->body->visit(call->function->body, ctx);

    ctx->variables = prevTable;

    freeObject((Object*)call);
    freeObject(calleeObj);
  } else if (calleeObj->type == OBJ_NATIVE_FUNCTION) {
    NativeFunction* nativeFunc = (NativeFunction*)calleeObj;
    
    if (fncallnode->argCount < nativeFunc->requiredArgCount || (!nativeFunc->isVariadic && fncallnode->argCount != nativeFunc->requiredArgCount)) {
      char buf[256];

      snprintf(buf, sizeof(buf), "Function '%s' expected atleast %zu argument%s, got %zu.", nativeFunc->name, nativeFunc->requiredArgCount, nativeFunc->requiredArgCount == 1 ? "" : "s", fncallnode->argCount);
      if (!*ctx->err) *ctx->err = initRuntimeError(fncallnode->start, fncallnode->end, ctx->filename, buf, ctx->sourcetext);
      
      freeObject(calleeObj);

      return NULL;
    }

    Object** evaluatedArgs = evaluateArgs(fncallnode->args, fncallnode->argCount, ctx);

    if (!evaluatedArgs) {
      freeObject(calleeObj);
      return NULL;
    }

    res = nativeFunc->function(evaluatedArgs, fncallnode->argCount);

    for (size_t i = 0; i < fncallnode->argCount; i++) 
      freeObject(evaluatedArgs[i]);

    free(evaluatedArgs);
    freeObject(calleeObj);
  } else {
    freeObject(calleeObj);
    return NULL;
  }

  if (res && res->type == OBJ_ERROR) {
    if (!*ctx->err) *ctx->err = initRuntimeError(fncallnode->start, fncallnode->end, ctx->filename, ((ProgramError*)res)->details, ctx->sourcetext);
    freeObject(res);
    return NULL;
  }

  if (res && (res->type == OBJ_BREAK || res->type == OBJ_CONTINUE)) {
    if (!*ctx->err) *ctx->err = initRuntimeError(fncallnode->start, fncallnode->end, ctx->filename, "Break/Continue outside of loop.", ctx->sourcetext);
    freeObject(res);
    return NULL;
  }
  
  if (res && res->type == OBJ_RETURN) {
    Object* val = ((Return*)res)->value;
    free(res);
    return val;
  }

  return res;
}

Object* visitImportNode(ASTNode* node, Interpretator* ctx) {
  ImportNode* import = (ImportNode*)node;

  char* name = import->filePath.val.s;

  for (size_t i = 0; stdlibModules[i]; i++) {
    if (strcmp(stdlibModules[i]->name, name) == 0) {
      stdlibModules[i]->init(ctx->variables);
      return (Object*)initInt(1);
    }
  }
  
  char *resolvedPath = resolveImportPath(ctx->filename, name);
  
  if (_DEBUG) printf("[debug] Resolved import path: %s\n", resolvedPath);

  char* fileContent = readFile(resolvedPath);

  if (!fileContent) {
    return NULL;
  }

  Lexer* lexer = initLexer(stringDup(name), fileContent);

  if (!lexer) {
    free(fileContent);
    return NULL;
  }

  size_t tokenAmount = 0;

  Token* tokens = makeTokensLexer(lexer, ctx->err, &tokenAmount);

  if (!tokens) {
    free(fileContent);

    return NULL;
  }

  Parser* parser = initParser(tokens, tokenAmount, ctx->err, name, fileContent);
  
  if (!parser) {
    freeTokens(tokens, tokenAmount);
    free(fileContent);

    return NULL;
  }

  ASTNode* ast = parseProgram(parser);

  if (!ast) {
    freeTokens(tokens, tokenAmount);
    free(fileContent);

    return NULL;
  }

  char *prevFilename = ctx->filename;
  char *prevFilecontent = ctx->sourcetext;
  
  ctx->sourcetext = fileContent;
  ctx->filename = lexer->filename;

  Object* result = ast->visit(ast, ctx);
  
  ctx->sourcetext = prevFilecontent;
  ctx->filename = prevFilename;

  if (!result) {
    freeTokens(tokens, tokenAmount);
    free(fileContent);

    return NULL;
  }

  Module* module = initModule(ast, lexer, parser, fileContent, tokens, tokenAmount);

  if (!module) {
    freeTokens(tokens, tokenAmount);
    free(parser);
    
    free(fileContent);

    return NULL;
  }

  setTable(ctx->variables, lexer->filename, (Object*)module, true);

  return result;
}

Object* visitReturnNode(ASTNode* node, Interpretator* ctx) {
  ReturnNode* ret = (ReturnNode*)node;

  Object* val = ret->expr->visit(ret->expr, ctx);
  if (!val) return NULL;

  if (val->type == OBJ_BREAK || val->type == OBJ_CONTINUE) {
    freeObject(val);
    if (!*ctx->err) *ctx->err = initRuntimeError(ret->start, ret->end, ctx->filename, "Cannot return a loop signal.", ctx->sourcetext);
    return NULL;
  }

  Return* retVal = initReturn(val);

  return (Object*)retVal;
}

Object* visitTryCatchNode(ASTNode* node, Interpretator* ctx) {
  TryCatchNode* tryCatch = (TryCatchNode*)node;

  Error* innerErr = NULL;

  Error **prevErr = ctx->err;
  ctx->err = &innerErr;

  Object* bodyRes = tryCatch->body->visit(tryCatch->body, ctx);
  
  ctx->err = prevErr;

  if (!innerErr) return bodyRes;

  if (bodyRes) freeObject(bodyRes);
  
  const char *details = innerErr->details;
  if (!details || details[0] == '\0') details = "Unknown Error.";

  Object* errObj = (Object*)initString((char*)details, (uint64_t)strlen(details));

  if (tryCatch->errIdentifier.val.s && errObj) {
    setTable(ctx->variables, tryCatch->errIdentifier.val.s, errObj, false);
  }
  
  freeError(innerErr);

  if (ctx->err) *ctx->err = NULL; // clear outer error

  return tryCatch->errHandler->visit(tryCatch->errHandler, ctx);
} 

Object* visitIndexAssignNode(ASTNode* node, Interpretator* ctx) {
  IndexAssignNode* ia = (IndexAssignNode*)node;
  Token ident = ia->targetIdent;

  Object* obj = getTable(ctx->variables, ident.val.s);

  if (!obj) {
    int len = snprintf(NULL, 0, "Undefined variable \"%s\"", ident.val.s);
    char *buffer = malloc(len + 1);

    snprintf(buffer, len + 1, "Undefined variable \"%s\"", ident.val.s); 
      
    if (*ctx->err == NULL) *ctx->err = initNameError(ident.start, ident.end, ctx->filename, buffer, ctx->sourcetext);
    free(buffer);

    return NULL;
  }

  if (obj->type != OBJ_LIST && obj->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Object of type '%s' is not indexable.", typeofobj(obj));

    if (*ctx->err == NULL) *ctx->err = initIndexError(ident.start, ident.end, ctx->filename, buf, ctx->sourcetext);
    return NULL;
  }

  Object* val = ia->value->visit(ia->value, ctx);

  if (!val) { // Err is already set 
    return NULL;
  }

  Object* index = ia->index->visit(ia->index, ctx);

  if (!index) { // Err is already set 
    freeObject(val);
    return NULL;
  }

  if (index->type != OBJ_NUMBER_INT) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected object of type 'int', received '%s'.", typeofobj(index));

    if (*ctx->err == NULL) *ctx->err = initTypeError(ident.start, ident.end, ctx->filename, buf, ctx->sourcetext);

    freeObject(val);
    freeObject(index);

    return NULL;
  }

  Number* idx = (Number*)index;

  if (idx->as.i < 0) {
    if (*ctx->err == NULL) *ctx->err = initValueError(ident.start, ident.end, ctx->filename, "Index out of range.", ctx->sourcetext);

    freeObject(val);
    freeObject(index);

    return NULL;
  }
  
  switch (obj->type) {
    case OBJ_STRING: {
      if (val->type != OBJ_STRING) {
        char buf[256];

        snprintf(buf, sizeof(buf), "Expected object of type 'string', received '%s'.", typeofobj(val));

        if (*ctx->err == NULL) *ctx->err = initValueError(ident.start, ident.end, ctx->filename, buf, ctx->sourcetext);
        
        freeObject(val);
        freeObject(index);

        return NULL;
      }

      String* valStr = (String*)val;

      if (strlen(valStr->value) != 1) {
        if (*ctx->err == NULL) *ctx->err = initValueError(ident.start, ident.end, ctx->filename, "Expected single-character string.", ctx->sourcetext);

        freeObject(val);
        freeObject(index);

        return NULL;
      }

      String* str = (String*)obj;
      uint64_t size = str->len;

      if ((uint64_t)idx->as.i > size) {
        if (*ctx->err == NULL) *ctx->err = initValueError(ident.start, ident.end, ctx->filename, "Index out of range.", ctx->sourcetext);

        freeObject(val);
        freeObject(index);

        return NULL;
      }

      str->value[idx->as.i] = valStr->value[0];
      freeObject(val);

      break;
    }

    case OBJ_LIST: {
      List* list = (List*)obj;
      uint64_t size = list->size;

      if ((uint64_t)idx->as.i > size) {
        if (*ctx->err == NULL) *ctx->err = initValueError(ident.start, ident.end, ctx->filename, "Index out of range.", ctx->sourcetext);

        freeObject(val);
        freeObject(index);

        return NULL;
      }
      
      freeObject(list->objects[idx->as.i]);
      list->objects[idx->as.i] = val;

      break;
    }

    default: { // technically unreachable, but here to suppress warning
      freeObject(val); 
      freeObject(index);

      return NULL;
    }
  }

  freeObject(index);
  
  return (Object*)initInt(1);
}

Object* visitForNode(ASTNode* node, Interpretator* ctx) {
  ForNode* fornode = (ForNode*)node;

  Object* iterable = fornode->iterable->visit(fornode->iterable, ctx);

  if (!iterable) { // Error is already set 
    return NULL;
  }

  if (iterable->type != OBJ_STRING && iterable->type != OBJ_LIST) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Object of type '%s' is not iterable.", typeofobj(iterable));

    if (!*ctx->err) *ctx->err = initTypeError(fornode->ident.start, fornode->ident.end, ctx->filename, buf, ctx->sourcetext);
    freeObject(iterable);

    return NULL;
  }

  // SymbolTable* env = createTable(1024, variables);

  // if (!env) {
  //  freeObject(iterable);
  //  return NULL;
  // }

  
  size_t len = iterable->type == OBJ_STRING ? ((String*)iterable)->len : ((List*)iterable)->size;

  for (size_t i = 0; i < len; i++) {
    Object* obj;

    if (iterable->type == OBJ_STRING) {
      char buf[] = {((String*)iterable)->value[i], '\0'};

      obj = (Object*)initString(buf, 1);
    } else {
      obj = copyObject(((List*)iterable)->objects[i]);
    }

    setTable(ctx->variables, fornode->ident.val.s, obj, false);

    Object* tmp = fornode->body->visit(fornode->body, ctx);
    if (!tmp) return NULL;

    if (tmp->type == OBJ_BREAK) {
      freeObject(tmp);
      break;
    }

    if (tmp->type == OBJ_CONTINUE) {
      freeObject(tmp);
      continue;
    }

    if (tmp && tmp->type == OBJ_RETURN) {
      freeObject(iterable);
      // freeTable(env);
      return tmp;
    }

    freeObject(tmp);
  }

  freeObject(iterable);
  // freeTable(env);

  return (Object*)initInt(1);
}

static int sCallDepth = 0;

Object* visitNode(ASTNode* node, Interpretator* ctx) {
  if (!ctx || !ctx->filename || !ctx->sourcetext || !ctx->err) return NULL;
  
  if (++sCallDepth > 4096) {
    --sCallDepth;
    return (Object*)initProgramError("Stack Overflow: call depth exceeded.");
  }

  Object* result = node->visit(node, ctx); 

  --sCallDepth;
  return result;
}

