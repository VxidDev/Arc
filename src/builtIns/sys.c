#include "../../include/builtIns/sys.h"
#include "../../include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

Object* builtIn_exit(Object** args, size_t argCount) {
  (void)argCount;

  Object* exitCode = args[0];

  if (exitCode->type != OBJ_NUMBER_INT) {
    char buf[256];

    snprintf(buf, sizeof(buf), "Expected object of type 'int', received '%s'.", typeofobj(exitCode));

    return (Object*)initProgramError(buf);
  }

  char buf[256];

  snprintf(buf, sizeof(buf), "@%" PRId64 "\n", ((Number*)exitCode)->as.i);

  return (Object*)initProgramError(buf); // will exit cleanly
}
