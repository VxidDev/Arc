#ifndef INTERPRETATOR_H
#define INTERPRETATOR_H

#include "object.h"
#include "error.h"
#include "symbol-table.h"
#include "node.h"

typedef struct Interpretator {
  char* filename;
  char* sourcetext;
  Error** err;
  SymbolTable* variables;
} Interpretator;

Interpretator* initInterpretator(char* filename, char* sourcetext, Error** err, SymbolTable* variables);

Object* visitNumberNode(ASTNode* node, Interpretator* ctx);
Object* visitStringNode(ASTNode* node, Interpretator* ctx);

Object* visitBinOpNode(ASTNode* node, Interpretator* ctx);
Object* visitUnaryOpNode(ASTNode* node, Interpretator* ctx);
Object* visitVarAccessNode(ASTNode* node, Interpretator* ctx);
Object* visitVarAssignNode(ASTNode* node, Interpretator* ctx); 
Object* visitProgramNode(ASTNode* node, Interpretator* ctx);
Object* visitIfNode(ASTNode* node, Interpretator* ctx); 
Object* visitListNode(ASTNode* n, Interpretator* ctx); 
Object* visitIndexNode(ASTNode* node, Interpretator* ctx);
Object* visitWhileNode(ASTNode* node, Interpretator* ctx);
Object* visitContinueNode(ASTNode* node, Interpretator* ctx);
Object* visitBreakNode(ASTNode* node, Interpretator* ctx);
Object* visitFunctionNode(ASTNode* node, Interpretator* ctx); 
Object* visitFunctionCallNode(ASTNode* node, Interpretator* ctx);
Object* visitImportNode(ASTNode* node, Interpretator* ctx);
Object* visitReturnNode(ASTNode* node, Interpretator* ctx);
Object* visitTryCatchNode(ASTNode* node, Interpretator* ctx);
Object* visitIndexAssignNode(ASTNode* node, Interpretator* ctx);
Object* visitForNode(ASTNode* node, Interpretator* ctx); 

Object* visitNode(ASTNode* node, Interpretator* ctx);

#endif // INTERPRETATOR_H
