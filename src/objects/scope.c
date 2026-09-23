#include "../../include/object.h"
#include "../../include/compiler.h"

#include <stdlib.h>

Scope* initScope(Chunk* chunk) {
  if (!chunk) return NULL;

  Scope* scope = malloc(sizeof(Scope));

  if (!scope) return NULL;

  scope->base.type = OBJ_SCOPE;
  scope->base.isStatic = false;

  scope->chunk = chunk;

  return scope;
}
