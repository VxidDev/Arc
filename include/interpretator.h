#ifndef INTERPRETATOR_H
#define INTERPRETATOR_H

#include "object.h"
#include "error.h"

#include "node.h"

Number* visitNode(ASTNode* node, char *filename, Error **err);

#endif // INTERPRETATOR_H
