#ifndef INTERPRETATOR_H
#define INTERPRETATOR_H

#include "object.h"
#include "error.h"
#include "symbol-table.h"
#include "node.h"

Object* visitNode(ASTNode* node, char *filename, Error **err, SymbolTable* variables);

#endif // INTERPRETATOR_H
