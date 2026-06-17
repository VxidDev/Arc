#include "../include/node.h"
#include "../include/token.h"
#include "../include/utils.h"

#include "../include/memarena.h"

#include <stdlib.h>
#include <string.h>

NumberNode* initNumberNode(Token token) {
  if (token.type == TOK_EOF) return NULL;

  NumberNode* node = arenaAlloc(parseArena, sizeof(NumberNode));

  if (!node) return NULL;

  node->base.type = NODE_NUMBER;
  node->token = token;

  return node;
}

ForNode* initForNode(Token forTok, Token identifier, ASTNode* iterable, ASTNode *body) {
  if (!body || !iterable) return NULL;

  ForNode* node = arenaAlloc(parseArena, sizeof(ForNode));

  if (!node) return NULL;

  node->base.type = NODE_FOR;

  node->forTok = forTok;
  node->ident = identifier;
  node->iterable = iterable;
  node->body = body;

  return node;
}

FunctionCallNode* initFunctionCallNode(ASTNode* callee, ASTNode **args, size_t argCount, Position start, Position end) {
  if (!callee || !args) return NULL;

  FunctionCallNode* fncallNode = arenaAlloc(parseArena, sizeof(FunctionCallNode));

  if (!fncallNode) return NULL;
  
  fncallNode->base.type = NODE_FUNCTION_CALL;
  
  fncallNode->callee = callee;
  fncallNode->args = args;
  fncallNode->argCount = argCount;

  fncallNode->start = start;
  fncallNode->end = end;

  return fncallNode;
}

TryCatchNode* initTryCatchNode(Position tryStart, Position catchEnd, Token errIdentifier, ASTNode* body, ASTNode* errHandler) {
  if (!body || !errHandler) return NULL;

  TryCatchNode* node = arenaAlloc(parseArena, sizeof(TryCatchNode));

  if (!node) return NULL;
    
  node->base.type = NODE_TRYCATCH;
  
  node->tryStart = tryStart;
  node->catchEnd = catchEnd;
  node->errIdentifier = errIdentifier;

  node->body = body;
  node->errHandler = errHandler;

  return node;
}

ContinueNode* initContinueNode(Token tok) {
  if (tok.type == TOK_EOF) return NULL;

  ContinueNode* node = arenaAlloc(parseArena, sizeof(ContinueNode));

  if (!node) return NULL;
  
  node->base.type = NODE_CONTINUE;
  node->tok = tok;

  return node;
}

BreakNode* initBreakNode(Token tok) {
  if (tok.type == TOK_EOF) return NULL;

  BreakNode* node = arenaAlloc(parseArena, sizeof(BreakNode));

  if (!node) return NULL;
  
  node->base.type = NODE_BREAK;
  node->tok = tok;

  return node;
}

FunctionNode* initFunctionNode(ASTNode* body, char *name, char **params, size_t paramCount, Position start, Position end) {
  if (!body) return NULL;

  FunctionNode* node = arenaAlloc(parseArena, sizeof(FunctionNode));

  if (!node) return NULL;
  
  node->base.type = NODE_FUNCTION;

  node->body = body;
  node->name = stringDup(name);

  if (!node->name) return NULL;

  node->params = params;
  node->paramCount = paramCount;
  node->start = start;
  node->end = end;

  return node;
}

ListNode* initListNode(Token startBracket, Token endBracket, ASTNode** objects, uint64_t size, uint64_t capacity) {
  if (startBracket.type == TOK_EOF || endBracket.type == TOK_EOF || !objects) return NULL;

  ListNode* list = arenaAlloc(parseArena, sizeof(ListNode));

  if (!list) return NULL;

  list->base.type = NODE_LIST;
  
  list->startBracket = startBracket;
  list->endBracket = endBracket;
  list->objects = objects;

  list->size = size;
  list->capacity = capacity;

  return list;
}

ProgramNode* initProgramNode(ASTNode **statements, size_t count) {
  ProgramNode *node = arenaAlloc(parseArena, sizeof(ProgramNode));
  if (!node) return NULL;

  node->base.type = NODE_PROGRAM;
  
  node->statements = statements;
  node->count = count;

  return node;
}

StringNode* initStringNode(Token token) {
  if (token.type == TOK_EOF) return NULL;

  StringNode* node = arenaAlloc(parseArena, sizeof(StringNode));

  if (!node) return NULL;

  node->base.type = NODE_STRING;
  
  node->token = token;
  node->len = strlen(token.val.s);

  return node;
}

IndexNode* initIndexNode(ASTNode* target, ASTNode* index, Position start, Position end) {
  if (!target || !index) return NULL;

  IndexNode* node = arenaAlloc(parseArena, sizeof(IndexNode));

  if (!node) return NULL;

  node->base.type = NODE_INDEX;
  
  node->target = target;
  node->index = index;

  node->start = start;
  node->end = end;

  return node;
}

PropertyAssignNode* initPropertyAssignNode(ASTNode* target, Token field, ASTNode* value, Position start, Position end) {
  if (!target || !value) return NULL;

  PropertyAssignNode* node = arenaAlloc(parseArena, sizeof(PropertyAssignNode));

  if (!node) return NULL;
  
  node->base.type = NODE_PROPERTYASSIGN;

  node->target = target;
  node->field = field;
  node->value = value;
  node->start = start;
  node->end = end;

  return node;
}

IndexAssignNode* initIndexAssignNode(ASTNode* target, ASTNode* index, ASTNode* value, Position start, Position end) {
  if (!index || !value) return NULL;

  IndexAssignNode* node = arenaAlloc(parseArena, sizeof(IndexAssignNode));

  if (!node) return NULL;

  node->base.type = NODE_INDEXASSIGN;
  
  node->target = target;
  node->index = index;
  node->value = value;
  node->start = start;
  node->end = end;

  return node;
}

WhileNode* initWhileNode(ASTNode* condition, ASTNode* body, Position start, Position end) {
  if (!condition || !body) return NULL;

  WhileNode* node = arenaAlloc(parseArena, sizeof(WhileNode));

  if (!node) return NULL;

  node->base.type = NODE_WHILE;
  
  node->condition = condition;
  node->body = body;

  node->start = start;
  node->end = end;

  return node;
}

BinOpNode* initBinOpNode(ASTNode *leftNode, Token operTok, ASTNode *rightNode) {
  if (!leftNode || operTok.type == TOK_EOF || !rightNode) return NULL;

  BinOpNode* node = arenaAlloc(parseArena, sizeof(BinOpNode));

  if (!node) return NULL;

  node->base.type = NODE_BINOP;

  node->leftNode = leftNode;
  node->operTok = operTok;
  node->rightNode = rightNode;

  return node;
}

UnaryOpNode* initUnaryOpNode(Token operTok, ASTNode* node) {
  if (operTok.type == TOK_EOF || !node) return NULL;

  UnaryOpNode* unaryNode = arenaAlloc(parseArena, sizeof(UnaryOpNode));

  if (!unaryNode) return NULL;

  unaryNode->base.type = NODE_UNARYOP;
  
  unaryNode->operTok = operTok;
  unaryNode->node = node;

  return unaryNode;
}

VarAssignNode* initVarAssignNode(char *identifier, ASTNode* value, Position start) {
  if (!identifier || !value) return NULL;

  VarAssignNode* varAssignNode = arenaAlloc(parseArena, sizeof(VarAssignNode));

  if (!varAssignNode) return NULL;

  varAssignNode->identifier = stringDup(identifier);

  if (!varAssignNode->identifier) return NULL;

  varAssignNode->base.type = NODE_VARASSIGN;
  varAssignNode->value = value;

  varAssignNode->start = start;

  return varAssignNode;
}

VarAccessNode* initVarAccessNode(Token token) {
  if (token.type == TOK_EOF) return NULL;

  VarAccessNode* node = arenaAlloc(parseArena, sizeof(VarAccessNode));

  if (!node) return NULL;

  node->base.type = NODE_VARACCESS;
  node->token = token;
  return node;
}

ImportNode* initImportNode(Token filePath) {
  if (filePath.type == TOK_EOF) return NULL;

  ImportNode* node = arenaAlloc(parseArena, sizeof(ImportNode));
  
  if (!node) return NULL;

  node->base.type = NODE_IMPORT;
  node->filePath = filePath;

  return node;
} 

ReturnNode* initReturnNode(Position start, Position end, ASTNode* expr) {
  if (!expr) return NULL;
  
  ReturnNode* node = arenaAlloc(parseArena, sizeof(ReturnNode));

  if (!node) return NULL;

  node->base.type = NODE_RETURN;
  node->expr = expr;
  node->start = start;
  node->end = end;

  return node;
}

IfNode* initIfNode(ASTNode* condition, ASTNode* thenExpr, ASTNode** elifConds, ASTNode** elifExprs, size_t elifCount, ASTNode* elseExpr) {
  if (!condition || !thenExpr) return NULL;

  IfNode* node = arenaAlloc(parseArena, sizeof(IfNode));
  if (!node) return NULL;

  node->base.type = NODE_IF;
  node->condition = condition;
  node->thenExpr = thenExpr;
  node->elifConds = elifConds;
  node->elifExprs = elifExprs;
  node->elifCount = elifCount;
  node->elseExpr = elseExpr;

  return node;
}

ClassNode* initClassNode(Token identifier, ASTNode* body, Position start, Position end) {
  if (!body) return NULL;

  ClassNode* node = arenaAlloc(parseArena, sizeof(ClassNode));

  node->base.type = NODE_CLASS;
  node->body = body;
  node->identifier = identifier;
  node->start = start;
  node->end = end;

  return node;
}

PropertyAccessNode* initPropertyAccessNode(ASTNode* target, Token field, Position start, Position end) {
  if (!target) return NULL;

  PropertyAccessNode* node = arenaAlloc(parseArena, sizeof(PropertyAccessNode));

  node->base.type = NODE_PROPERTYACCESS;
  node->target = target;
  node->field = field;
  node->start = start;
  node->end = end;

  return node;
} 

Position getNodeStart(ASTNode *node) {
  if (!node) return (Position){0,0,0};

  switch (node->type) {
    case NODE_NUMBER: return ((NumberNode*)node)->token.start;
    case NODE_STRING: return ((StringNode*)node)->token.start;
    case NODE_VARACCESS: return ((VarAccessNode*)node)->token.start;
    case NODE_VARASSIGN: return ((VarAssignNode*)node)->start;
    case NODE_BINOP: return getNodeStart(((BinOpNode*)node)->leftNode);
    case NODE_UNARYOP: return ((UnaryOpNode*)node)->operTok.start;
    case NODE_IF: return getNodeStart(((IfNode*)node)->condition);
    case NODE_LIST: return ((ListNode*)node)->startBracket.start;
    case NODE_INDEX: return ((IndexNode*)node)->start;
    case NODE_WHILE: return ((WhileNode*)node)->start;
    case NODE_FUNCTION: return ((FunctionNode*)node)->start;
    case NODE_FUNCTION_CALL: return ((FunctionCallNode*)node)->start;
    case NODE_IMPORT: return ((ImportNode*)node)->filePath.start;
    case NODE_RETURN: return ((ReturnNode*)node)->start;
    case NODE_TRYCATCH: return ((TryCatchNode*)node)->tryStart;
    case NODE_BREAK: return ((BreakNode*)node)->tok.start;
    case NODE_CONTINUE: return ((ContinueNode*)node)->tok.start;
    case NODE_INDEXASSIGN: return ((IndexAssignNode*)node)->start;
    case NODE_FOR: return ((ForNode*)node)->forTok.start;
    case NODE_CLASS: return ((ClassNode*)node)->start;
    case NODE_PROPERTYASSIGN: return ((PropertyAssignNode*)node)->start;
    case NODE_PROPERTYACCESS: return getNodeStart(((PropertyAccessNode*)node)->target);
    case NODE_PROGRAM: {
      ProgramNode *p = (ProgramNode*)node;
      return p->count > 0 ? getNodeStart(p->statements[0]) : (Position){0,0,0};
    }

    default: return (Position){0,0,0};
  }
}

Position getNodeEnd(ASTNode *node) {
  if (!node) return (Position){0,0,0};

  switch (node->type) {
    case NODE_NUMBER: return ((NumberNode*)node)->token.end;
    case NODE_STRING: return ((StringNode*)node)->token.end;
    case NODE_VARACCESS: return ((VarAccessNode*)node)->token.end;
    case NODE_VARASSIGN: return getNodeEnd(((VarAssignNode*)node)->value);
    case NODE_BINOP: return getNodeEnd(((BinOpNode*)node)->rightNode);
    case NODE_UNARYOP: return getNodeEnd(((UnaryOpNode*)node)->node);
    case NODE_IF: return getNodeEnd(((IfNode*)node)->condition);
    case NODE_LIST: return ((ListNode*)node)->endBracket.end;
    case NODE_INDEX: return ((IndexNode*)node)->end;
    case NODE_WHILE: return ((WhileNode*)node)->end;
    case NODE_FUNCTION: return ((FunctionNode*)node)->end;
    case NODE_FUNCTION_CALL: return ((FunctionCallNode*)node)->end;
    case NODE_IMPORT: return ((ImportNode*)node)->filePath.end;
    case NODE_RETURN: return ((ReturnNode*)node)->end;
    case NODE_TRYCATCH: return ((TryCatchNode*)node)->catchEnd;
    case NODE_BREAK: return ((BreakNode*)node)->tok.end;
    case NODE_CONTINUE: return ((ContinueNode*)node)->tok.end;
    case NODE_INDEXASSIGN: return ((IndexAssignNode*)node)->end;
    case NODE_FOR: return getNodeEnd(((ForNode*)node)->body);
    case NODE_CLASS: return ((ClassNode*)node)->end;
    case NODE_PROPERTYACCESS: return ((PropertyAccessNode*)node)->end;
    case NODE_PROPERTYASSIGN: return ((PropertyAssignNode*)node)->end;
    case NODE_PROGRAM: {
      ProgramNode *p = (ProgramNode*)node;
      return p->count > 0 ? getNodeEnd(p->statements[p->count-1]) : (Position){0,0,0};
    }
    default: return (Position){0,0,0};
  }
}
