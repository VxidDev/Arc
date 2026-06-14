#include "../../include/object.h"
#include "../../include/mempool.h"
#include "../../include/memarena.h"
#include "../../include/symbol-table.h"
#include "../../include/utils.h"

Class* initClass(ClassNode* node) {
  if (!node) return NULL;

  Class* class = arenaAlloc(objectArena, sizeof(Class));
  if (!class) return NULL;

  class->base.type = OBJ_CLASS;
  class->base.isStatic = false;
  class->chunk = NULL;
  class->name = stringDup(node->identifier.val.s);
  class->maxLocals = 0;

  return class;
}

Instance* initInstance(Class* klass, SymbolTable* globals) {
  Instance* instance = poolAlloc(instancePool);
  if (!instance) return NULL;

  instance->base.type = OBJ_INSTANCE;
  instance->base.isStatic = true;

  instance->klass = klass;

  instance->fields = createTable(128, globals);

  if (!instance->fields) {
    poolFree(instancePool, instance);
    return NULL;
  }

  return instance;
}
