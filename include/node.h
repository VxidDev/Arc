#ifndef NODE_H
#define NODE_H

#include "token.h"
#include "error.h"
#include <stddef.h>

typedef enum {
  NODE_NUMBER,
  NODE_BINOP,
  NODE_UNARYOP,
  NODE_VARASSIGN,
  NODE_VARACCESS,
  NODE_STRING,
  NODE_PROGRAM,
  NODE_IF,
  NODE_LIST,
  NODE_INDEX,
  NODE_WHILE,
  NODE_FUNCTION,
  NODE_FUNCTION_CALL,
  NODE_IMPORT,
  NODE_RETURN,
  NODE_TRYCATCH,
  NODE_BREAK,
  NODE_CONTINUE,
  NODE_INDEXASSIGN,
  NODE_FOR,
  NODE_CLASS,
  NODE_PROPERTYACCESS,
  NODE_PROPERTYASSIGN
} NodeType;

typedef struct Interpretator Interpretator;
typedef struct Object Object;
typedef struct ASTNode ASTNode;

typedef Object* (*VisitFn)(ASTNode*, Interpretator*);

typedef struct ASTNode {
  NodeType type;
} ASTNode;

typedef struct NumberNode {
  ASTNode base;
  Token token;
} NumberNode;

typedef struct StringNode {
  ASTNode base;
  Token token;
  uint64_t len;
} StringNode;

typedef struct ListNode {
  ASTNode base;
  ASTNode** objects;
  uint64_t size, capacity;
  Token startBracket;
  Token endBracket;
} ListNode;

typedef struct VarAccessNode {
  ASTNode base;
  Token token;
} VarAccessNode;

typedef struct VarAssignNode {
  ASTNode base;
  char *identifier;
  ASTNode *value;
  Position start;
} VarAssignNode;

typedef struct IndexNode {
  ASTNode base;
  ASTNode* target;
  ASTNode* index;

  Position start, end;
} IndexNode;

typedef struct BinOpNode {
  ASTNode base;

  ASTNode *leftNode;
  Token operTok;
  ASTNode *rightNode;
} BinOpNode;

typedef struct UnaryOpNode {
  ASTNode base;

  Token operTok;
  ASTNode* node;
} UnaryOpNode;

typedef struct IfNode {
  ASTNode base;
  ASTNode* condition;
  ASTNode* thenExpr;
  ASTNode** elifConds;
  ASTNode** elifExprs;
  size_t elifCount;
  ASTNode* elseExpr;
} IfNode;

typedef struct WhileNode {
  ASTNode base;
  ASTNode* condition;
  ASTNode* body;
  Position start, end;
} WhileNode;

typedef struct FunctionNode {
  ASTNode base;
  char *name;
  char **params;
  size_t paramCount;
  ASTNode* body;
  Position start, end;
} FunctionNode;

typedef struct FunctionCallNode {
  ASTNode base;
  ASTNode* callee;
  ASTNode** args;
  size_t argCount;

  Position start, end;
} FunctionCallNode;

typedef struct ImportNode {
  ASTNode base;
  Token filePath;
} ImportNode; 

typedef struct {
  ASTNode base;
  ASTNode **statements;
  size_t count;
} ProgramNode;

typedef struct ReturnNode {
  ASTNode base;
  ASTNode* expr;
  Position start, end;
} ReturnNode;

typedef struct TryCatchNode {
  ASTNode base;
  Token errIdentifier;
  Position tryStart, catchEnd;
  ASTNode *body, *errHandler;
} TryCatchNode;

typedef struct BreakNode {
  ASTNode base;
  Token tok;
} BreakNode;

typedef struct ContinueNode {
  ASTNode base;
  Token tok;
} ContinueNode;

typedef struct IndexAssignNode {
  ASTNode base;
  ASTNode* target; 
  ASTNode* value;
  ASTNode* index;

  Position start, end;
} IndexAssignNode;

typedef struct ForNode {
  ASTNode base;

  Token forTok;
  Token ident;
  ASTNode* iterable;
  ASTNode* body;
} ForNode;

typedef struct ClassNode {
  ASTNode base;

  Token identifier;
  ASTNode* body;

  Position start, end;
} ClassNode;

typedef struct PropertyAccessNode {
  ASTNode base;
  ASTNode* target;
  Token field;
  Position start, end;
} PropertyAccessNode;

typedef struct PropertyAssignNode {
  ASTNode base; 
  ASTNode* target;
  Token field;
  ASTNode* value;
  Position start, end;
} PropertyAssignNode;

PropertyAssignNode* initPropertyAssignNode(ASTNode* target, Token field, ASTNode* value, Position start, Position end);
NumberNode* initNumberNode(Token token);
StringNode* initStringNode(Token token);
ClassNode* initClassNode(Token identifier, ASTNode* body, Position start, Position end);
BinOpNode* initBinOpNode(ASTNode *leftNode, Token operTok, ASTNode *rightNode);
UnaryOpNode* initUnaryOpNode(Token operTok, ASTNode* node);
VarAccessNode* initVarAccessNode(Token token);
VarAssignNode* initVarAssignNode(char *identifier, ASTNode* value, Position start);
IfNode* initIfNode(ASTNode* condition, ASTNode* thenExpr, ASTNode** elifConds, ASTNode** elifExprs, size_t elifCount, ASTNode* elseExpr);
ListNode* initListNode(Token startBracket, Token endBracket, ASTNode** objects, uint64_t size, uint64_t capacity);
IndexNode* initIndexNode(ASTNode* target, ASTNode* index, Position start, Position end);
ProgramNode* initProgramNode(ASTNode** statements, size_t count);
FunctionNode* initFunctionNode(ASTNode* body, char *name, char **params, size_t paramCount, Position start, Position end);
FunctionCallNode *initFunctionCallNode(ASTNode *callee, ASTNode **args, size_t argCount, Position start, Position end);
ImportNode* initImportNode(Token filePath);
TryCatchNode* initTryCatchNode(Position tryStart, Position catchEnd, Token errIdentifier, ASTNode* body, ASTNode* errHandler);
ReturnNode* initReturnNode(Position start, Position end, ASTNode* expr);
BreakNode* initBreakNode(Token tok);
ContinueNode* initContinueNode(Token tok);
IndexAssignNode* initIndexAssignNode(ASTNode* target, ASTNode* index, ASTNode* value, Position start, Position end);
WhileNode* initWhileNode(ASTNode* condition, ASTNode* body, Position start, Position end);
ForNode* initForNode(Token forTok, Token ident, ASTNode* iterable, ASTNode* body);
PropertyAccessNode* initPropertyAccessNode(ASTNode* target, Token field, Position start, Position end);

Position getNodeStart(ASTNode* node);
Position getNodeEnd(ASTNode* node);

#endif // NODE_H
