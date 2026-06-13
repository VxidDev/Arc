#include "../../include/object.h"
#include "../../include/utils.h"

#include <stdlib.h>

ProgramError* initProgramError(char *details) {
  if (!details) return NULL;

  ProgramError* error = malloc(sizeof(ProgramError));

  if (!error) return NULL;

  error->base.type = OBJ_ERROR;
  error->base.isStatic = false;

  error->details = stringDup(details);

  return error;
}
