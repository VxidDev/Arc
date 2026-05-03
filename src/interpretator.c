#include "../include/node.h"
#include "../include/interpretator.h"
#include "../include/object.h"
#include "../include/symbol-table.h"
#include "../include/utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Object* visitNumberNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables) {
  if (!filename || !err) return NULL;

  NumberNode* numNode = (NumberNode*)node;

  if (!numNode->token) {
    // dereferencing will cause segfault -> if (*err == NULL) *err = initValueError(copyPosition(numNode->token->start), copyPosition(numNode->token->end), filename, "Expected TOK_FLOAT or TOK_INT token, received NULL");
    return NULL;
  }
  
  if (strcmp(numNode->token->type, TOK_FLOAT) == 0) {
    return (Object*)initFloat(*(double*)numNode->token->value);
  }

  return (Object*)initInt(*(long*)numNode->token->value);
}

Object* _stringBinOp(BinOpNode* binOper, Object* srcObj, char op, Object* destObj, char *filename, Error **err, SymbolTable* variables) {
  if (op != '+') {
    char buffer[256];
    
    snprintf(buffer, sizeof(buffer), "Unsupported operand types for '%c': string %c string", op, op); 

    freeObject(srcObj);
    freeObject(destObj);
    
    if (*err == NULL) *err = initTypeError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, buffer);
    return NULL;
  }

  Object* out = (Object*)addString((String*)srcObj, (String*)destObj);

  freeObject(srcObj);
  freeObject(destObj);

  return out;
}

Object* visitBinOpNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables) {
  if (!filename || !err || !node) return NULL;

  BinOpNode* binOper = (BinOpNode*)node;

  char op;

  if (strcmp(binOper->operTok->type, TOK_PLUS) == 0) op = '+';
  else if (strcmp(binOper->operTok->type, TOK_MINUS) == 0) op = '-';
  else if (strcmp(binOper->operTok->type, TOK_MUL) == 0) op = '*';
  else if (strcmp(binOper->operTok->type, TOK_DIV) == 0) op = '/';
  else if (strcmp(binOper->operTok->type, TOK_POW) == 0) op = '^';
  else op = '?';

  Object *srcObj = visitNode(binOper->leftNode, filename, err, variables);
  Object *destObj = visitNode(binOper->rightNode, filename, err, variables);
  
  if (!srcObj || !destObj) {
    if (srcObj) free(srcObj);
    if (destObj) free(destObj);
    return NULL;
  }

  const char* leftType =
    srcObj->type == OBJ_NUMBER_INT ? "int" :
    srcObj->type == OBJ_NUMBER_FLOAT ? "float" :
    srcObj->type == OBJ_STRING ? "string" : "unknown";

  const char* rightType =
    destObj->type == OBJ_NUMBER_INT ? "int" :
    destObj->type == OBJ_NUMBER_FLOAT ? "float" :
    destObj->type == OBJ_STRING ? "string" : "unknown";

  if (srcObj->type != OBJ_NUMBER_INT && srcObj->type != OBJ_NUMBER_FLOAT) {
    if (srcObj->type == OBJ_STRING && destObj->type == OBJ_STRING) return _stringBinOp(binOper, srcObj, op, destObj, filename, err, variables);

    char buffer[256];
    
    snprintf(buffer, sizeof(buffer), "Unsupported operand types for '%c': %s %c %s", op, leftType, op, rightType); 

    free(srcObj);
    free(destObj);
    
    if (*err == NULL) *err = initTypeError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, buffer);
    return NULL;
  }

  if (destObj->type != OBJ_NUMBER_INT && destObj->type != OBJ_NUMBER_FLOAT) { 
    char buffer[256];
    
    snprintf(buffer, sizeof(buffer), "Unsupported operand types for '%c': %s %c %s", op, leftType, op, rightType); 
    
    free(srcObj);
    free(destObj);

    if (*err == NULL) *err = initTypeError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, buffer);
    return NULL;
  }

  Number* src = (Number*)srcObj;
  Number* dest = (Number*)destObj;

  if (!src || !dest) {
    if (*err == NULL) *err = initValueError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, "Expected TOK_FLOAT or TOK_INT token, received NULL");
    if (src) free(src);
    if (dest) free(dest);
    return NULL;
  } 
  
  EvalResultNumber output;

  if (strcmp(binOper->operTok->type, TOK_PLUS) == 0) {
    output = addNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_MINUS) == 0) {
    output = subNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_MUL) == 0) {
    output = mulNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_DIV) == 0) {
    output = divNumber(dest, src);
  } else if (strcmp(binOper->operTok->type, TOK_POW) == 0) {
    output = powNumber(dest, src);
  }

  if (output.err) {
    if (output.err == ERR_NULL) {
      if (*err == NULL) *err = initValueError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, "Expected TOK_FLOAT or TOK_INT token, received NULL");
    } else if (output.err == ERR_DIV_BY_ZERO) {
      if (*err == NULL) *err = initValueError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, "Division by zero");
    } else if (output.err == ERR_TYPE) {
      if (*err == NULL) *err = initTypeError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, "Incompatible type for binary operation");
    } else { if (*err == NULL) *err = initValueError(copyPosition(binOper->operTok->start), copyPosition(binOper->operTok->end), filename, "Unknown error."); }

    free(src);
    free(dest);

    return NULL;
  }

  free(dest);
  free(src);

  return (Object*)output.num;
}

Object* visitUnaryOpNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables) {
  UnaryOpNode* unaryOper = (UnaryOpNode*)node;

  char op;

  if (strcmp(unaryOper->operTok->type, TOK_PLUS) == 0) op = '+';
  else if (strcmp(unaryOper->operTok->type, TOK_MINUS) == 0) op = '-';
  else if (strcmp(unaryOper->operTok->type, TOK_MUL) == 0) op = '*';
  else if (strcmp(unaryOper->operTok->type, TOK_DIV) == 0) op = '/';
  else if (strcmp(unaryOper->operTok->type, TOK_POW) == 0) op = '^';
  else op = '?';

  Object* numberObj = (Object*)visitNode(unaryOper->node, filename, err, variables); 

  if (!numberObj) {
    if (*err == NULL) *err = initValueError(copyPosition(unaryOper->operTok->start), copyPosition(unaryOper->operTok->end), filename, "Expected Number* result, received NULL.");
    return NULL;
  }

  const char* type =
    numberObj->type == OBJ_NUMBER_INT ? "int" :
    numberObj->type == OBJ_NUMBER_FLOAT ? "float" :
    numberObj->type == OBJ_STRING ? "string" : "unknown";

  if (numberObj->type != OBJ_NUMBER_INT && numberObj->type != OBJ_NUMBER_FLOAT) {
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "Can't apply unary operation '%c' to %s", op, type); 

    free(numberObj);
    
    if (*err == NULL) *err = initTypeError(copyPosition(unaryOper->operTok->start), copyPosition(unaryOper->operTok->end), filename, buffer);
    return NULL;
  }

  Number* number = (Number*)numberObj;
  
  EvalResultNumber output = {0};

  if (op == '-') {
    Number* negOne = initInt(-1);
    output = mulNumber(number, negOne);
    free(number);
    free(negOne);
  } else {
    if (*err == NULL) *err = initValueError(copyPosition(unaryOper->operTok->start), copyPosition(unaryOper->operTok->end), filename, "Unknown unary operator");
    free(number);
    return NULL;
  }

  return (Object*)output.num;
}

Object* visitVarAccessNode(ASTNode* node, char *filename, Error** err, SymbolTable* variables) {
  if (!node) return NULL;
  
  VarAccessNode* va = (VarAccessNode*)node;
  char *varName = (char*)va->token->value;

  if (!varName) return NULL;
  
  Object* stored = (Object*)getTable(variables, varName);

  if (!stored) {
    int len = snprintf(NULL, 0, "Undefined variable \"%s\"", varName);
    char *buffer = malloc(len + 1);

    snprintf(buffer, len + 1, "Undefined variable \"%s\"", varName);

    if (*err == NULL) *err = initNameError(copyPosition(va->token->start), copyPosition(va->token->end), filename, buffer);
    free(buffer);

    return NULL;
  }

  return copyObject(stored); 
}

Object* visitVarAssignNode(ASTNode* node, char *filename, Error** err, SymbolTable* variables) {
  if (!node) return NULL;

  VarAssignNode* va = (VarAssignNode*)node;

  Object* value = visitNode(va->value, filename, err, variables);

  if (!value) return NULL;
  
  setTable(variables, va->identifier, value);

  return value;
}

Object* visitStringNode(ASTNode* node, char *filename, Error** err, SymbolTable* variables) {
  if (!node) return NULL;

  StringNode* str = (StringNode*)node;

  return (Object*)initString((char*)str->token->value);
}

Object* visitNode(ASTNode* node, char *filename, Error** err, SymbolTable* variables) {
  if (!node || !filename || !err) return NULL;

  switch (node->type) {
    case NODE_NUMBER: return visitNumberNode(node, filename, err, variables); 
    case NODE_BINOP: return visitBinOpNode(node, filename, err, variables);
    case NODE_UNARYOP: return visitUnaryOpNode(node, filename, err, variables);
    case NODE_VARACCESS: return visitVarAccessNode(node, filename, err, variables);
    case NODE_VARASSIGN: return visitVarAssignNode(node, filename, err, variables);
    case NODE_STRING: return visitStringNode(node, filename, err, variables);
    default: return NULL;
  }
}
