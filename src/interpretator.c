#include "../include/node.h"
#include "../include/interpretator.h"
#include "../include/object.h"
#include "../include/symbol-table.h"
#include "../include/lexer.h"
#include "../include/parser.h"

#include "../include/utils.h"

#include "../include/repl/readfile.h"
#include "../include/repl/repl.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Object* visitNumberNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables) {
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

Object* visitBinOpNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables) {
  BinOpNode* binOper = (BinOpNode*)node;

  TokType opType = binOper->operTok.type;

  const char* op = binOpStr[opType];
  if (!op) op = "?";

  ASTNode* leftNode = binOper->leftNode;
  ASTNode* rightNode = binOper->rightNode;

  Object *srcObj = leftNode->visit(leftNode, filename, sourcetext, err, variables);
  Object *destObj = rightNode->visit(rightNode, filename, sourcetext, err, variables);

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
    return _stringBinOp(binOper, srcObj , op, destObj, leftType, rightType, filename, sourcetext, err);

  if (!isSrcNum || !isDestNum) {
    _initBinOpTypeErr(binOper->operTok.start, binOper->operTok.end, op, leftType, rightType, filename, sourcetext, err);

    freeObject(srcObj);
    freeObject(destObj);

    return NULL;
  }

  Number* src = (Number*)srcObj;
  Number* dest = (Number*)destObj;

  if (!src || !dest) {
    if (!*err) *err = initValueError(binOper->operTok.start, binOper->operTok.end, filename, "Expected TOK_FLOAT or TOK_INT token, received NULL", sourcetext);

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

  switch (opType) {
    case TOK_PLUS: output = addNumber(dest, src); break;
    case TOK_MINUS: output = subNumber(dest, src); break;
    case TOK_MUL: output = mulNumber(dest, src); break;
    case TOK_DIV: output = divNumber(dest, src); break;
    case TOK_POW: output = powNumber(dest, src); break;
    case TOK_EE: output = isEqualNumber(dest, src); break;
    case TOK_LT: output = isLessThanNumber(dest, src); break;
    case TOK_GT: output = isGreaterThanNumber(dest, src); break;
    case TOK_LTE: output = isLessThanEqualNumber(dest, src); break;
    case TOK_GTE: output = isGreaterThanEqualNumber(dest, src); break;
    case TOK_NE: output = isNotEqualNumber(dest, src); break;
    case TOK_AND: output = andNumber(dest, src); break;
    case TOK_OR: output = orNumber(dest, src); break;
    default: output = -1;
  }

  switch (output) {
    case ERR_NONE:
      break;
    case ERR_NULL:
      if (!*err) *err = initValueError(binOper->operTok.start, binOper->operTok.end, filename, "Expected TOK_FLOAT or TOK_INT token, received NULL", sourcetext);
      break;
    case ERR_DIV_BY_ZERO:
      if (!*err) *err = initValueError(binOper->operTok.start, binOper->operTok.end, filename, "Division by zero", sourcetext);
      break;
    case ERR_TYPE:
      if (!*err) *err = initTypeError(binOper->operTok.start, binOper->operTok.end, filename, "Incompatible type for binary operation", sourcetext);
      break;
    default:
      if (!*err) *err = initValueError(binOper->operTok.start, binOper->operTok.end, filename, "Unknown error.", sourcetext);
      break;
  }

  freeObject((Object*)src);

  if (output != ERR_NONE) {
    freeObject((Object*)dest);
    return NULL;
  }

  return (Object*)dest;
}

Object* visitUnaryOpNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables) {
  UnaryOpNode* unaryOper = (UnaryOpNode*)node;

  char* op;

  switch (unaryOper->operTok.type) {
    case TOK_PLUS: op = "+"; break;
    case TOK_MINUS: op = "-"; break;
    case TOK_NOT: op = "NOT"; break;
    default: op = "?"; break;
  }

  Object* numberObj = (Object*)unaryOper->node->visit(unaryOper->node, filename, sourcetext, err, variables);

  if (!numberObj) {
    if (!*err) *err = initValueError(unaryOper->operTok.start, unaryOper->operTok.end, filename, "Expected Number* result, received NULL.", sourcetext);
    return NULL;
  }

  const char* type = typeofobj(numberObj);

  if (numberObj->type != OBJ_NUMBER_INT && numberObj->type != OBJ_NUMBER_FLOAT) {
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "Can't apply unary operation '%s' to %s", op, type);

    freeObject(numberObj);

    if (!*err) *err = initTypeError(unaryOper->operTok.start, unaryOper->operTok.end, filename, buffer, sourcetext);
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
      if (!*err) *err = initSyntaxError(unaryOper->operTok.start, unaryOper->operTok.end, filename, "Unknown unary operator.", sourcetext);
      return NULL;
  }

  return NULL;
}

Object* visitVarAccessNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables) {
  VarAccessNode* va = (VarAccessNode*)node;
  char *varName = va->token.val.s;

  Object* stored = (Object*)getTable(variables, varName);

  if (!stored) {
    int len = snprintf(NULL, 0, "Undefined variable \"%s\"", varName);
    char *buffer = malloc(len + 1);

    snprintf(buffer, len + 1, "Undefined variable \"%s\"", varName);

    if (!*err) *err = initNameError(va->token.start, va->token.end, filename, buffer, sourcetext);
    free(buffer);

    return NULL;
  }

  return copyObject(stored);
}

Object* visitVarAssignNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables) {
  VarAssignNode* va = (VarAssignNode*)node;

  Object* value = va->value->visit(va->value, filename, sourcetext, err, variables);

  if (!value) return NULL;

  setTable(variables, va->identifier, value, true);

  return value;
}

Object* visitStringNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables) {
  StringNode* str = (StringNode*)node;

  return (Object*)initString(str->token.val.s, str->len);
}

Object* visitContinueNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables) {
  return (Object*)initContinue();
}

Object* visitBreakNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables) {
  return (Object*)initBreak();
}

Object* visitProgramNode(ASTNode* node, char *filename, char* sourcetext, Error **err, SymbolTable* variables) {
  ProgramNode* prog = (ProgramNode*)node;

  Object* last = NULL;

  for (size_t i = 0; i < prog->count; i++) {
    freeObject(last);
    last = prog->statements[i]->visit(prog->statements[i], filename, sourcetext, err, variables);

    if (!last) return NULL; // error already set
    
    if (last->type == OBJ_RETURN || last->type == OBJ_BREAK || last->type == OBJ_CONTINUE) {
      return last;
    }
  }

  return last;
}

Object* visitIfNode(ASTNode* n, char *filename, char *sourcetext, Error **err, SymbolTable* variables) {
  if (!n) return NULL;

  IfNode* node = (IfNode*)n;

  Object* condition = node->condition->visit(node->condition, filename, sourcetext, err, variables);

  if (!condition) return NULL;
  if (condition->type != OBJ_NUMBER_INT) return NULL;

  Number* cond = (Number*)condition;

  if (cond->as.i != 0) {
    freeObject(condition);
    Object* obj = node->thenExpr->visit(node->thenExpr, filename, sourcetext, err, variables);

    return obj;
  }

  freeObject(condition);

  for (size_t i = 0; i < node->elifCount; i++) {
    Object* elifVal = node->elifConds[i]->visit(node->elifConds[i], filename, sourcetext, err, variables);
    if (!elifVal) return NULL;

    if (elifVal->type != OBJ_NUMBER_INT) return NULL;

    if (((Number*)elifVal)->as.i != 0) {
      freeObject(elifVal);
      Object* obj = node->elifExprs[i]->visit(node->elifExprs[i], filename, sourcetext, err, variables);

      return obj;
    }

    freeObject(elifVal);
  }

  // Fall through to ELSE if present
  if (node->elseExpr) {
    Object* obj = node->elseExpr->visit(node->elseExpr, filename, sourcetext, err, variables);

    return obj;
  }

  return (Object*)initInt(0);
}

Object* visitListNode(ASTNode* node, char* filename, char *sourcetext, Error** err, SymbolTable* variables) {
  ListNode* listNode = (ListNode*)node;
  
  Object *objs[listNode->capacity];

  for (uint64_t i = 0; i < listNode->size; i++) {
    objs[i] = listNode->objects[i]->visit(listNode->objects[i], filename, sourcetext, err, variables);
  }

  List* list = initList(objs, listNode->size, listNode->capacity);

  return (Object*)list;
}

Object* visitIndexNode(ASTNode* node, char* filename, char *sourcetext, Error **err, SymbolTable* variables) {
  IndexNode* idx = (IndexNode*)node;

  const char* targetType; 
  const char* idxType; 

  Object* target = idx->target->visit(idx->target, filename, sourcetext, err, variables);

  if (!target) {
    return NULL;
  }

  targetType = typeofobj(target);

  Object* index = idx->index->visit(idx->index, filename, sourcetext, err, variables);

  if (!index) {
    return NULL;
  }

  idxType = typeofobj(index);

  if (target->type == OBJ_NUMBER_INT || target->type == OBJ_NUMBER_FLOAT) {
    char buff[256];

    snprintf(buff, sizeof(buff), "Object of type \"%s\" is not subscriptable.", targetType);

    if (!*err) *err = initTypeError(idx->start, idx->end, filename, buff, sourcetext);

    freeObject(target);
    freeObject(index);

    return NULL;
  }

  if (index->type != OBJ_NUMBER_INT) {
    char buff[256];

    snprintf(buff, sizeof(buff), "Cannot use a value of type \"%s\" as an index.", idxType);

    if (!*err) *err = initTypeError(idx->start, idx->end, filename, buff, sourcetext);

    freeObject(target);
    freeObject(index);

    return NULL;
  } 

  if (target->type == OBJ_STRING) {
    String* str = (String*)target;
    Number* id = (Number*)index;

    if (id->as.i < 0 || (uint64_t)id->as.i >= str->len) {
      if (!*err) *err = initIndexError(idx->start, idx->end, filename, "Index out of range.", sourcetext);

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
    if (*err == NULL) *err = initIndexError(idx->start, idx->end, filename, "Index out of range.", sourcetext);

    freeObject(target);
    freeObject(index);

    return NULL;
  } 

  Object* obj = copyObject(list->objects[id->as.i]);

  freeObject(target);
  freeObject(index);

  return obj;
}

Object* visitWhileNode(ASTNode* node, char* filename, char *sourcetext, Error** err, SymbolTable* variables) {
  WhileNode* whileNode = (WhileNode*)node;

  ASTNode* condNode = whileNode->condition;
  ASTNode* bodyNode = whileNode->body;

  while (true) {  
    Object* cond = condNode->visit(condNode, filename, sourcetext, err, variables);

    if (!cond) return NULL;

    if (cond->type != OBJ_NUMBER_INT) {
      if (*err == NULL) *err = initTypeError(whileNode->start, whileNode->end, filename, "Condition must be evaluated to integer type object.", sourcetext);

      freeObject(cond);

      return NULL;
    }

    int value = ((Number*)cond)->as.i;
    freeObject(cond);

    if (value == 0) break;

    Object* tmp = bodyNode->visit(bodyNode, filename, sourcetext, err, variables);
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

Object* visitFunctionNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables) {
  FunctionNode* func = (FunctionNode*)node;
  Object* funcObj = (Object*)initFunction(func);

  if (!funcObj) {
    return NULL;
  }

  setTable(variables, func->name, funcObj, false);

  return (Object*)initInt(1); // true
}

static Object** evaluateArgs(ASTNode** args, size_t argCount, char* filename, char *sourcetext, Error** err, SymbolTable* variables) {
  Object** evaluatedArgs = malloc(sizeof(Object*) * argCount);
  if (!evaluatedArgs) return NULL;

  for (size_t i = 0; i < argCount; i++) {
    evaluatedArgs[i] = args[i]->visit(args[i], filename, sourcetext, err, variables);
    if (!evaluatedArgs[i]) {
      for (size_t j = 0; j < i; j++) freeObject(evaluatedArgs[j]);
      free(evaluatedArgs);
      return NULL;
    }
  }
  return evaluatedArgs;
}

Object* visitFunctionCallNode(ASTNode* node, char* filename, char *sourcetext, Error **err, SymbolTable* variables) {
  FunctionCallNode* fncallnode = (FunctionCallNode*)node;
  Object* calleeObj = fncallnode->callee->visit(fncallnode->callee, filename, sourcetext, err, variables);

  if (!calleeObj) return NULL;

  Object* res = NULL;

  if (calleeObj->type == OBJ_FUNCTION) {
    FunctionCall* call = initFunctionCall(fncallnode, calleeObj, variables, filename, sourcetext, err);

    if (!call) {
      freeObject(calleeObj);
      return NULL;
    }

    Function* func = (Function*)call->function;

    if (call->argCount != func->paramCount) {
      char buf[256];

      snprintf(buf, sizeof(buf), "Function '%s' expected %zu arguments, got %zu.", func->name, func->paramCount, call->argCount);
      if (!*err) *err = initRuntimeError(fncallnode->start, fncallnode->end, filename, buf, sourcetext);

      freeObject((Object*)call);
      freeObject(calleeObj);
      return NULL;
    }

    res = call->function->body->visit(call->function->body, filename, sourcetext, err, call->env);

    freeObject((Object*)call);
    freeObject(calleeObj);
  } else if (calleeObj->type == OBJ_NATIVE_FUNCTION) {
    NativeFunction* nativeFunc = (NativeFunction*)calleeObj;
    
    if (fncallnode->argCount < nativeFunc->requiredArgCount || (!nativeFunc->isVariadic && fncallnode->argCount != nativeFunc->requiredArgCount)) {
      char buf[256];

      snprintf(buf, sizeof(buf), "Function '%s' expected atleast %zu argument%s, got %zu.", nativeFunc->name, nativeFunc->requiredArgCount, nativeFunc->requiredArgCount == 1 ? "" : "s", fncallnode->argCount);
      if (!*err) *err = initRuntimeError(fncallnode->start, fncallnode->end, filename, buf, sourcetext);
      
      freeObject(calleeObj);

      return NULL;
    }

    Object** evaluatedArgs = evaluateArgs(fncallnode->args, fncallnode->argCount, filename, sourcetext, err, variables);

    if (!evaluatedArgs) {
      freeObject(calleeObj);
      return NULL;
    }

    res = nativeFunc->function(evaluatedArgs, fncallnode->argCount);

    for (size_t i = 0; i < fncallnode->argCount; i++) freeObject(evaluatedArgs[i]);

    free(evaluatedArgs);
    freeObject(calleeObj);
  } else {
    freeObject(calleeObj);
    return NULL;
  }

  if (res && res->type == OBJ_ERROR) {
    if (!*err) *err = initRuntimeError(fncallnode->start, fncallnode->end, filename, ((ProgramError*)res)->details, sourcetext);
    freeObject(res);
    return NULL;
  }

  if (res && (res->type == OBJ_BREAK || res->type == OBJ_CONTINUE)) {
    if (!*err) *err = initRuntimeError(fncallnode->start, fncallnode->end, filename, "Break/Continue outside of loop.", sourcetext);
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

Object* visitImportNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables) {
  ImportNode* import = (ImportNode*)node;

  char* name = import->filePath.val.s;

  for (size_t i = 0; stdlibModules[i]; i++) {
    if (strcmp(stdlibModules[i]->name, name) == 0) {
        stdlibModules[i]->init(variables);
        return (Object*)initInt(1);
    }
  }

  char* fileContent = readFile(name);

  if (!fileContent) {
    return NULL;
  }

  Lexer* lexer = initLexer(stringDup(name), fileContent);

  if (!lexer) {
    free(fileContent);
    return NULL;
  }

  size_t tokenAmount = 0;

  Token* tokens = makeTokensLexer(lexer, err, &tokenAmount);

  if (!tokens) {
    free(fileContent);

    return NULL;
  }

  Parser* parser = initParser(tokens, tokenAmount, err, name, fileContent);
  
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

  Object* result = ast->visit(ast, lexer->filename, fileContent, err, variables);

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

  setTable(variables, lexer->filename, (Object*)module, true);

  return result;
}

Object* visitReturnNode(ASTNode* node, char* filename, char *sourcetext, Error** err, SymbolTable* variables) {
  ReturnNode* ret = (ReturnNode*)node;

  Object* val = ret->expr->visit(ret->expr, filename, sourcetext, err, variables);
  if (!val) return NULL;

  if (val->type == OBJ_BREAK || val->type == OBJ_CONTINUE) {
    freeObject(val);
    if (!*err) *err = initRuntimeError(ret->start, ret->end, filename, "Cannot return a loop signal.", sourcetext);
    return NULL;
  }

  Return* retVal = initReturn(val);

  return (Object*)retVal;
}

Object* visitTryCatchNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables) {
  TryCatchNode* tryCatch = (TryCatchNode*)node;

  Error* innerErr = NULL;
  Object* bodyRes = tryCatch->body->visit(tryCatch->body, filename, sourcetext, &innerErr, variables);

  if (!innerErr) return bodyRes;

  if (bodyRes) freeObject(bodyRes);
  
  const char *details = innerErr->details;
  if (!details || details[0] == '\0') details = "Unknown Error.";

  Object* errObj = (Object*)initString((char*)details, (uint64_t)strlen(details));

  if (tryCatch->errIdentifier.val.s && errObj) {
    setTable(variables, tryCatch->errIdentifier.val.s, errObj, false);
  }
  
  freeError(innerErr);

  if (err) *err = NULL; // clear outer error

  return tryCatch->errHandler->visit(tryCatch->errHandler, filename, sourcetext, err, variables);
} 

Object* visitIndexAssignNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables) {
  IndexAssignNode* ia = (IndexAssignNode*)node;
  Token ident = ia->targetIdent;

  Object* obj = (Object*)getTable(variables, ident.val.s);

  if (!obj) {
    int len = snprintf(NULL, 0, "Undefined variable \"%s\"", ident.val.s);
    char *buffer = malloc(len + 1);

    snprintf(buffer, len + 1, "Undefined variable \"%s\"", ident.val.s); 
      
    if (*err == NULL) *err = initNameError(ident.start, ident.end, filename, buffer, sourcetext);
    free(buffer);

    return NULL;
  }

  if (obj->type != OBJ_LIST && obj->type != OBJ_STRING) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Object of type '%s' is not indexable.", typeofobj(obj));

    if (*err == NULL) *err = initIndexError(ident.start, ident.end, filename, buf, sourcetext);
    return NULL;
  }

  Object* val = ia->value->visit(ia->value, filename, sourcetext, err, variables);

  if (!val) { // Err is already set 
    return NULL;
  }

  Object* index = ia->index->visit(ia->index, filename, sourcetext, err, variables);

  if (!index) { // Err is already set 
    freeObject(val);
    return NULL;
  }

  if (index->type != OBJ_NUMBER_INT) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected object of type 'int', received '%s'.", typeofobj(index));

    if (*err == NULL) *err = initTypeError(ident.start, ident.end, filename, buf, sourcetext);

    freeObject(val);
    freeObject(index);

    return NULL;
  }

  Number* idx = (Number*)index;

  if (idx->as.i < 0) {
    if (*err == NULL) *err = initValueError(ident.start, ident.end, filename, "Index out of range.", sourcetext);

    freeObject(val);
    freeObject(index);

    return NULL;
  }
  
  switch (obj->type) {
    case OBJ_STRING: {
      if (val->type != OBJ_STRING) {
        char buf[256];

        snprintf(buf, sizeof(buf), "Expected object of type 'string', received '%s'.", typeofobj(val));

        if (*err == NULL) *err = initValueError(ident.start, ident.end, filename, buf, sourcetext);
        
        freeObject(val);
        freeObject(index);

        return NULL;
      }

      String* valStr = (String*)val;

      if (strlen(valStr->value) != 1) {
        if (*err == NULL) *err = initValueError(ident.start, ident.end, filename, "Expected single-character string.", sourcetext);

        freeObject(val);
        freeObject(index);

        return NULL;
      }

      String* str = (String*)obj;
      uint64_t size = str->len;

      if ((uint64_t)idx->as.i > size) {
        if (*err == NULL) *err = initValueError(ident.start, ident.end, filename, "Index out of range.", sourcetext);

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
        if (*err == NULL) *err = initValueError(ident.start, ident.end, filename, "Index out of range.", sourcetext);

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

Object* visitForNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables) {
  ForNode* fornode = (ForNode*)node;

  Object* iterable = fornode->iterable->visit(fornode->iterable, filename, sourcetext, err, variables);

  if (!iterable) { // Error is already set 
    return NULL;
  }

  if (iterable->type != OBJ_STRING && iterable->type != OBJ_LIST) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Object of type '%s' is not iterable.", typeofobj(iterable));

    if (!*err) *err = initTypeError(fornode->ident.start, fornode->ident.end, filename, buf, sourcetext);
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

    setTable(variables, fornode->ident.val.s, obj, false);

    Object* tmp = fornode->body->visit(fornode->body, filename, sourcetext, err, variables);
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

Object* visitNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables) {
  if (!node || !filename || !err || !variables) return NULL;
  
  if (++sCallDepth > 4096) {
    --sCallDepth;
    return (Object*)initProgramError("Stack Overflow: call depth exceeded.");
  }

  Object* result = NULL;

  result = node->visit(node, filename, sourcetext, err, variables); 

  --sCallDepth;
  return result;
}
