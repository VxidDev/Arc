#ifndef INTERPRETATOR_H
#define INTERPRETATOR_H

#include "object.h"
#include "error.h"
#include "symbol-table.h"
#include "node.h"

Object* visitNumberNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables);
Object* visitStringNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables);

Object* visitBinOpNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables);
Object* visitUnaryOpNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables);
Object* visitVarAccessNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables);
Object* visitVarAssignNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables); 
Object* visitProgramNode(ASTNode* node, char *filename, char* sourcetext, Error **err, SymbolTable* variables);
Object* visitIfNode(ASTNode* n, char *filename, char *sourcetext, Error **err, SymbolTable* variables); 
Object* visitListNode(ASTNode* node, char* filename, char *sourcetext, Error** err, SymbolTable* variables); 
Object* visitIndexNode(ASTNode* node, char* filename, char *sourcetext, Error** err, SymbolTable* variables);
Object* visitWhileNode(ASTNode* node, char* filename, char *sourcetext, Error** err, SymbolTable* variables);
Object* visitContinueNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables);
Object* visitBreakNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables);
Object* visitFunctionNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables); 
Object* visitFunctionCallNode(ASTNode* node, char* filename, char *sourcetext, Error **err, SymbolTable* variables);
Object* visitImportNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables);
Object* visitReturnNode(ASTNode* node, char* filename, char *sourcetext, Error** err, SymbolTable* variables);
Object* visitTryCatchNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables);
Object* visitIndexAssignNode(ASTNode* node, char *filename, char *sourcetext, Error** err, SymbolTable* variables);
Object* visitForNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables); 

Object* visitNode(ASTNode* node, char *filename, char *sourcetext, Error **err, SymbolTable* variables);

#endif // INTERPRETATOR_H
