#include "../include/utils.h"

#include <string.h>
#include <stdlib.h>

char *stringDup(char *s) {
  unsigned long len = strlen(s) + 1;
  char *sDup = malloc(len * sizeof(char));
  
  memcpy(sDup, s, len * sizeof(char));

  return sDup;
}
