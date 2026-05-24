#include "../include/node.h"
#include "../include/token.h"
#include "../include/utils.h"

#include <stdlib.h>
#include <string.h>

NumberNode* initNumberNode(Token* token) {
  if (!token) return NULL;

  NumberNode* node = malloc(sizeof(NumberNode));

  if (!node) return NULL;

  node->base.type = NODE_NUMBER;
  node->token = token;

  return node;
}

FunctionCallNode* initFunctionCallNode(ASTNode* callee, ASTNode **args, size_t argCount) {
  if (!callee || !args) return NULL;

  FunctionCallNode* fncallNode = malloc(sizeof(FunctionCallNode));

  if (!fncallNode) return NULL;
  
  fncallNode->base.type = NODE_FUNCTION_CALL;

  fncallNode->callee = callee;
  fncallNode->args = args;
  fncallNode->argCount = argCount;

  return fncallNode;
}

FunctionNode* initFunctionNode(ASTNode* body, char *name, char **params, size_t paramCount) {
  if (!body) return NULL;

  FunctionNode* node = malloc(sizeof(FunctionNode));

  if (!node) return NULL;
  
  node->base.type = NODE_FUNCTION;
  node->body = body;
  node->name = stringDup(name);

  if (!node->name) {
    free(node);
    return NULL;
  }

  node->params = params;
  node->paramCount = paramCount;

  return node;
}

ListNode* initListNode(Token* startBracket, Token* endBracket, ASTNode** objects, uint64_t size, uint64_t capacity) {
  if (!startBracket || !endBracket || !objects) return NULL;

  ListNode* list = malloc(sizeof(ListNode));

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
  ProgramNode *node = malloc(sizeof(ProgramNode));
  if (!node) return NULL;

  node->base.type = NODE_PROGRAM;
  node->statements = statements;
  node->count = count;

  return node;
}

StringNode* initStringNode(Token* token) {
  if (!token) return NULL;

  StringNode* node = malloc(sizeof(StringNode));

  if (!node) return NULL;

  node->base.type = NODE_STRING;
  node->token = token;
  node->len = strlen(token->val.s);

  return node;
}

IndexNode* initIndexNode(ASTNode* target, ASTNode* index, Position start, Position end) {
  if (!target || !index) return NULL;

  IndexNode* node = malloc(sizeof(IndexNode));

  if (!node) return NULL;

  node->base.type = NODE_INDEX;
  node->target = target;
  node->index = index;

  node->start = start;
  node->end = end;

  return node;
}

WhileNode* initWhileNode(ASTNode* condition, ASTNode* body, Position start, Position end) {
  if (!condition || !body) return NULL;

  WhileNode* node = malloc(sizeof(WhileNode));

  if (!node) return NULL;

  node->base.type = NODE_WHILE;
  node->condition = condition;
  node->body = body;

  node->start = start;
  node->end = end;

  return node;
}

BinOpNode* initBinOpNode(ASTNode *leftNode, Token *operTok, ASTNode *rightNode) {
  if (!leftNode || !operTok || !rightNode) return NULL;

  BinOpNode* node = malloc(sizeof(BinOpNode));

  if (!node) return NULL;

  node->base.type = NODE_BINOP;

  node->leftNode = leftNode;
  node->operTok = operTok;
  node->rightNode = rightNode;

  return node;
}

UnaryOpNode* initUnaryOpNode(Token* operTok, ASTNode* node) {
  if (!operTok || !node) return NULL;

  UnaryOpNode* unaryNode = malloc(sizeof(UnaryOpNode));

  if (!unaryNode) return NULL;

  unaryNode->base.type = NODE_UNARYOP;
  unaryNode->operTok = operTok;
  unaryNode->node = node;

  return unaryNode;
}

VarAssignNode* initVarAssignNode(char *identifier, ASTNode* value) {
  if (!identifier || !value) return NULL;

  VarAssignNode* varAssignNode = malloc(sizeof(VarAssignNode));

  if (!varAssignNode) return NULL;

  varAssignNode->identifier = stringDup(identifier);

  if (!varAssignNode->identifier) {
    free(varAssignNode);
    return NULL;
  }

  varAssignNode->base.type = NODE_VARASSIGN;
  varAssignNode->value = value;

  return varAssignNode;
}

VarAccessNode* initVarAccessNode(Token* token) {
  if (!token) return NULL;

  VarAccessNode* node = malloc(sizeof(VarAccessNode));

  if (!node) return NULL;

  node->base.type = NODE_VARACCESS;
  node->token = token;
  return node;
}

void freeVarAssignNode(VarAssignNode* node) {
  if (!node) return;

  if (node->identifier) free(node->identifier);
  if (node->value) freeAST(node->value);

  free(node);
}

void freeFunctionNode(FunctionNode* node) {
  if (!node) return;

  if (node->name) free(node->name);
  if (node->params) {
    for (size_t i = 0; i < node->paramCount; i++) {
      free(node->params[i]);
    }
    free(node->params);
  }
  if (node->body) freeAST(node->body);

  free(node);
}

void freeFunctionCallNode(FunctionCallNode* node) {
  if (!node) return;

  if (node->args) {
    for (size_t i = 0; i < node->argCount; i++) {
      freeAST(node->args[i]);
    }

    free(node->args);
  }

  if (node->callee) freeAST(node->callee);
  free(node);
}

IfNode* initIfNode(ASTNode* condition, ASTNode* thenExpr, ASTNode** elifConds, ASTNode** elifExprs, size_t elifCount, ASTNode* elseExpr) {
  if (!condition || !thenExpr) return NULL;

  IfNode* node = malloc(sizeof(IfNode));
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

void freeVarAccessNode(VarAccessNode* node) {
  if (!node) return;

  free(node);
}

void freeIndexNode(IndexNode* node) {
  if (!node) return;

  freeAST(node->target);
  freeAST(node->index);

  free(node);
}

void freeWhileNode(WhileNode* node) {
  if (!node) return;

  freeAST(node->condition);
  freeAST(node->body);

  free(node);
}

void freeListNode(ListNode* node) {
  if (!node) return;

  if (node->objects) {
    for (uint64_t i = 0; i < node->size; i++) {
      freeAST(node->objects[i]);
    }

    free(node->objects);
  }

  free(node);
}

void freeStringNode(StringNode* node) {
  if (!node) return;

  // if (node->token->val.s) free(node->token->val.s);
  free(node);
}

void freeNumberNode(NumberNode *node) {
    if (!node) return;

    free(node);
}

void freeBinOpNode(BinOpNode *node) {
    if (!node) return;

    freeAST(node->leftNode);
    freeAST(node->rightNode);

    free(node);
}

void freeUnaryOpNode(UnaryOpNode *node) {
    if (!node) return;

    freeAST(node->node);
    free(node);
}

void freeProgramNode(ProgramNode *node) {
  if (!node) return;

  if (node->statements) {
    for (size_t i = 0; i < node->count; i++) {
      if (node->statements[i]) {
        freeAST(node->statements[i]);
      }
    }

    free(node->statements);
  }

  free(node);
}

void freeIfNode(IfNode* node) {
  if (!node) return;

  freeAST(node->condition);
  freeAST(node->thenExpr);

  for (size_t i = 0; i < node->elifCount; i++) {
    freeAST(node->elifConds[i]);
    freeAST(node->elifExprs[i]);
  }

  free(node->elifConds);
  free(node->elifExprs);
  freeAST(node->elseExpr);
  free(node);
}

void freeAST(ASTNode *node) {
  if (!node) return;

  switch (node->type) {
    case NODE_NUMBER:
      freeNumberNode((NumberNode*)node);
      break;

    case NODE_BINOP:
      freeBinOpNode((BinOpNode*)node);
      break;

    case NODE_UNARYOP:
      freeUnaryOpNode((UnaryOpNode*)node);
      break;

    case NODE_VARASSIGN:
      freeVarAssignNode((VarAssignNode*)node);
      break;

    case NODE_VARACCESS:
      freeVarAccessNode((VarAccessNode*)node);
      break;

    case NODE_STRING:
      freeStringNode((StringNode*)node);
      break;

    case NODE_PROGRAM:
      freeProgramNode((ProgramNode*)node);
      break;

    case NODE_IF:
      freeIfNode((IfNode*)node);
      break;

    case NODE_LIST:
      freeListNode((ListNode*)node);
      break;

    case NODE_INDEX:
      freeIndexNode((IndexNode*)node);
      break;

    case NODE_WHILE:
      freeWhileNode((WhileNode*)node);
      break;

    case NODE_FUNCTION:
      freeFunctionNode((FunctionNode*)node);
      break;

    case NODE_FUNCTION_CALL:
      freeFunctionCallNode((FunctionCallNode*)node);
      break;
  }
}
