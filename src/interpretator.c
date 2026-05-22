#include "../include/node.h"
#include "../include/interpretator.h"
#include "../include/object.h"
#include "../include/symbol-table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Object* visitNumberNode(ASTNode* node, char *filename, Error **err) {
  if (!filename || !err) return NULL;

  NumberNode* numNode = (NumberNode*)node;

  if (!numNode->token) {
    // dereferencing will cause segfault -> if (*err == NULL) *err = initValueError(copyPosition(numNode->token->start), copyPosition(numNode->token->end), filename, "Expected TOK_FLOAT or TOK_INT token, received NULL");
    return NULL;
  }

  if (numNode->token->type == TOK_FLOAT) {
    return (Object*)initFloat(numNode->token->val.f);
  }

  return (Object*)initInt(numNode->token->val.i);
}

Object* _stringBinOp(BinOpNode* binOper, Object* srcObj, char* op, Object* destObj, const char *leftType, const char *rightType, char *filename, Error **err) {
  Object* out = NULL;

  if (strcmp(op, "+") == 0) {
    if (srcObj->type != OBJ_STRING || destObj->type != OBJ_STRING) {
      char buffer[256];

      snprintf(buffer, sizeof(buffer), "Unsupported operand types for '+': %s + %s", leftType, rightType);

      freeObject(srcObj);
      freeObject(destObj);

      if (*err == NULL) *err = initTypeError(binOper->operTok->start, binOper->operTok->end, filename, buffer);
      return NULL;
    }

    out = (Object*)addString((String*)srcObj, (String*)destObj);
  } else if (strcmp(op, "*") == 0) {
    if (srcObj->type != OBJ_STRING || destObj->type != OBJ_NUMBER_INT) {
      char buffer[256];

      snprintf(buffer, sizeof(buffer), "Unsupported operand types for '*': %s * %s", leftType, rightType);

      freeObject(srcObj);
      freeObject(destObj);

      if (*err == NULL) *err = initTypeError(binOper->operTok->start, binOper->operTok->end, filename, buffer);
      return NULL;
    }

    out = (Object*)mulString((String*)srcObj, (Number*)destObj);
  } else if (strcmp(op, "==") == 0) {
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
  } else {
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "Unsupported operand types for '%s': %s %s %s", op, leftType, op, rightType);

    freeObject(srcObj);
    freeObject(destObj);

    if (*err == NULL) *err = initTypeError(binOper->operTok->start, binOper->operTok->end, filename, buffer);
    return NULL;
  }

  freeObject(srcObj);
  freeObject(destObj);

  return out;
}

Object* visitBinOpNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables) {
  if (!filename || !err || !node) return NULL;

  BinOpNode* binOper = (BinOpNode*)node;

  char *op;

  switch (binOper->operTok->type) {
    case TOK_PLUS: op = "+"; break;
    case TOK_MINUS: op = "-"; break;
    case TOK_MUL: op = "*"; break;
    case TOK_DIV: op = "/"; break;
    case TOK_POW: op = "^"; break;
    case TOK_EQ: op = "="; break;
    case TOK_EE: op = "=="; break;
    case TOK_NE: op = "!="; break;
    case TOK_LT: op = "<"; break;
    case TOK_GT: op = ">"; break;
    case TOK_LTE: op = "<="; break;
    case TOK_GTE: op = ">="; break;
    case TOK_AND: op = "AND"; break;
    case TOK_OR: op = "OR"; break;
    default: op = "?"; break;
  }

  Object *srcObj = visitNode(binOper->leftNode, filename, err, variables);
  Object *destObj = visitNode(binOper->rightNode, filename, err, variables);

  if (!srcObj || !destObj) {
    if (srcObj) freeObject(srcObj);
    if (destObj) freeObject(destObj);
    return NULL;
  }

  const char* leftType;
  const char* rightType;

  switch (srcObj->type) {
    case OBJ_NUMBER_INT: leftType = "int"; break;
    case OBJ_NUMBER_FLOAT: leftType = "float"; break;
    case OBJ_STRING: leftType = "string"; break;
    default: leftType = "unknown"; break;
  }

  switch (destObj->type) {
    case OBJ_NUMBER_INT: rightType = "int"; break;
    case OBJ_NUMBER_FLOAT: rightType = "float"; break;
    case OBJ_STRING: rightType = "string"; break;
    default: rightType = "unknown"; break;
  }

  if (srcObj->type == OBJ_STRING || destObj->type == OBJ_STRING)
    return _stringBinOp(binOper, srcObj, op, destObj, leftType, rightType, filename, err);

  if ((srcObj->type != OBJ_NUMBER_INT && srcObj->type != OBJ_NUMBER_FLOAT) || (destObj->type != OBJ_NUMBER_INT && destObj->type != OBJ_NUMBER_FLOAT)) {
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "Unsupported operand types for '%s': %s %s %s", op, leftType, op, rightType);

    free(srcObj);
    free(destObj);

    if (*err == NULL) *err = initTypeError(binOper->operTok->start, binOper->operTok->end, filename, buffer);
    return NULL;
  }

  Number* src = (Number*)srcObj;
  Number* dest = (Number*)destObj;

  if (!src || !dest) {
    if (*err == NULL) *err = initValueError(binOper->operTok->start, binOper->operTok->end, filename, "Expected TOK_FLOAT or TOK_INT token, received NULL");
    if (src) free(src);
    if (dest) free(dest);
    return NULL;
  }

  ErrType output;

  switch (binOper->operTok->type) {
    case TOK_PLUS: output = addNumber(dest, src); break;
    case TOK_MINUS: output = subNumber(dest, src); break;
    case TOK_MUL: output = mulNumber(dest, src); break;
    case TOK_DIV: output = divNumber(dest, src); break;
    case TOK_POW: output = powNumber(dest, src); break;
    case TOK_EE: output = isEqualNumber(dest, src); break;
    case TOK_LT: output = isLessThanNumber(dest, src); break;
    case TOK_GT: output = isGreaterThanNumber(dest, src); break;
    case TOK_LTE: output = isLessThanEqualNumber(dest, src); break;
    case TOK_GTE: output = isGreaterThanNumber(dest, src); break;
    case TOK_NE: output = isNotEqualNumber(dest, src); break;
    case TOK_AND: output = andNumber(dest, src); break;
    case TOK_OR: output = orNumber(dest, src); break;
    default: output = -1;
  }

  switch (output) {
    case ERR_NONE:
      break;
    case ERR_NULL:
      if (*err == NULL) *err = initValueError(binOper->operTok->start, binOper->operTok->end, filename, "Expected TOK_FLOAT or TOK_INT token, received NULL");
      break;
    case ERR_DIV_BY_ZERO:
      if (*err == NULL) *err = initValueError(binOper->operTok->start, binOper->operTok->end, filename, "Division by zero");
      break;
    case ERR_TYPE:
      *err = initTypeError(binOper->operTok->start, binOper->operTok->end, filename, "Incompatible type for binary operation");
      break;
    default:
      if (*err == NULL) *err = initValueError(binOper->operTok->start, binOper->operTok->end, filename, "Unknown error.");
      break;
  }

  free(src);

  if (output != ERR_NONE) return NULL;

  return (Object*)dest;
}

Object* visitUnaryOpNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables) {
  UnaryOpNode* unaryOper = (UnaryOpNode*)node;

  char op;

  switch (unaryOper->operTok->type) {
    case TOK_PLUS: op = '+'; break;
    case TOK_MINUS: op = '-'; break;
    case TOK_MUL: op = '*'; break;
    case TOK_DIV: op = '/'; break;
    case TOK_POW: op = '^'; break;
    default: op = '?'; break;
  }

  Object* numberObj = (Object*)visitNode(unaryOper->node, filename, err, variables);

  if (!numberObj) {
    if (*err == NULL) *err = initValueError(unaryOper->operTok->start, unaryOper->operTok->end, filename, "Expected Number* result, received NULL.");
    return NULL;
  }

  const char* type;

  switch (numberObj->type) {
    case OBJ_NUMBER_INT: type = "int"; break;
    case OBJ_NUMBER_FLOAT: type = "float"; break;
    case OBJ_STRING: type = "string"; break;
    default: type = "unknown"; break;
  }

  if (numberObj->type != OBJ_NUMBER_INT && numberObj->type != OBJ_NUMBER_FLOAT) {
    char buffer[256];

    snprintf(buffer, sizeof(buffer), "Can't apply unary operation '%c' to %s", op, type);

    free(numberObj);

    if (*err == NULL) *err = initTypeError(unaryOper->operTok->start, unaryOper->operTok->end, filename, buffer);
    return NULL;
  }

  if (op == '-') {
    if (numberObj->type == OBJ_NUMBER_INT) {
      ((Number*)numberObj)->as.i *= -1;
      return numberObj;
    }

    ((Number*)numberObj)->as.f *= -1.0;
    return numberObj;
  } else {
    if (*err == NULL) *err = initValueError(unaryOper->operTok->start, unaryOper->operTok->end, filename, "Unknown unary operator");
    free(numberObj);
    return NULL;
  }

  return NULL;
}

Object* visitVarAccessNode(ASTNode* node, char *filename, Error** err, SymbolTable* variables) {
  if (!node) return NULL;

  VarAccessNode* va = (VarAccessNode*)node;
  char *varName = va->token->val.s;

  if (!varName) return NULL;

  Object* stored = (Object*)getTable(variables, varName);

  if (!stored) {
    int len = snprintf(NULL, 0, "Undefined variable \"%s\"", varName);
    char *buffer = malloc(len + 1);

    snprintf(buffer, len + 1, "Undefined variable \"%s\"", varName);

    if (*err == NULL) *err = initNameError(va->token->start, va->token->end, filename, buffer);
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

Object* visitStringNode(ASTNode* node) {
  if (!node) return NULL;

  StringNode* str = (StringNode*)node;

  return (Object*)initString(str->token->val.s);
}

Object* visitProgramNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables) {
  ProgramNode* prog = (ProgramNode*)node;

  Object* last = NULL;

  for (size_t i = 0; i < prog->count; i++) {
    if (last) {
      freeObject(last);
      last = NULL;
    }

    last = visitNode(prog->statements[i], filename, err, variables);

    if (!last) return NULL; // error already set
  }

  return last;
}

Object* visitIfNode(ASTNode* n, char *filename, Error **err, SymbolTable* variables) {
  if (!n) return NULL;

  IfNode* node = (IfNode*)n;

  Object* condition = visitNode(node->condition, filename, err, variables);

  if (!condition) return NULL;
  if (condition->type != OBJ_NUMBER_INT) return NULL;

  Number* cond = (Number*)condition;

  if (cond->as.i != 0) {
    free(cond);
    return visitNode(node->thenExpr, filename, err, variables);
  }

  free(cond);

  for (size_t i = 0; i < node->elifCount; i++) {
    Object* elifVal = visitNode(node->elifConds[i], filename, err, variables);
    if (!elifVal) return NULL;

    if (elifVal->type != OBJ_NUMBER_INT) return NULL;

    if (((Number*)elifVal)->as.i != 0) {
      free(elifVal);
      return visitNode(node->elifExprs[i], filename, err, variables);
    }

    free(elifVal);
  }

  // Fall through to ELSE if present
  if (node->elseExpr) {
    return visitNode(node->elseExpr, filename, err, variables);
  }

  return NULL;
}

Object* visitNode(ASTNode* node, char *filename, Error** err, SymbolTable* variables) {
  if (!node || !filename || !err) return NULL;

  switch (node->type) {
    case NODE_NUMBER: return visitNumberNode(node, filename, err);
    case NODE_BINOP: return visitBinOpNode(node, filename, err, variables);
    case NODE_UNARYOP: return visitUnaryOpNode(node, filename, err, variables);
    case NODE_VARACCESS: return visitVarAccessNode(node, filename, err, variables);
    case NODE_VARASSIGN: return visitVarAssignNode(node, filename, err, variables);
    case NODE_STRING: return visitStringNode(node);
    case NODE_PROGRAM: return visitProgramNode(node, filename, err, variables);
    case NODE_IF: return visitIfNode(node, filename, err, variables);
    default: return NULL;
  }
}
