#include "../include/utils.h"
#include "../include/error.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void freeError(Error* err) {
  if (!err) return;

  if (err->name) free(err->name);
  if (err->details) free(err->details);
  if (err->filename) free(err->filename);
  
  if (err->start) freePosition(err->start);
  if (err->end) freePosition(err->end);

  free(err);
}

Error* initError(Position* start, Position* end, char *name, char *filename, char *details) {
  if (!name || !details) return NULL;

  Error *error = malloc(sizeof(Error));

  if (!error) return NULL;

  error->name = stringDup(name);

  if (!error->name) {
    free(error);
    return NULL;
  }
  
  error->details = stringDup(details);

  if (!error->details) {
    free(error->name);
    free(error);
    return NULL;
  }
  
  error->filename = stringDup(filename);

  if (!error->filename) {
    free(error->name);
    free(error->details);
    free(error);
    return NULL;
  }

  error->start = start;
  error->end = end;

  return error;
}

Error *initIllegalCharError(Position* start, Position* end, char *filename, char *details) {
  return initError(start, end, "Illegal Character", filename, details);
}

Error *initSyntaxError(Position* start, Position* end, char *filename, char* details) {
  return initError(start, end, "Syntax Error", filename, details);
}

Error *initValueError(Position* start, Position* end, char *filename, char* details) {
  return initError(start, end, "Value Error", filename, details);
}

Error *initLexerError(Position* start, Position* end, char *filename, char* details) {
  return initError(start, end, "Lexer Error", filename, details);
}

Error *initSemanticError(Position* start, Position *end, char *filename, char* details) {
  return initError(start, end, "Semantic Error", filename, details); 
}

char* errorAsString(const Error* error) {
  if (!error || !error->name || !error->details || !error->filename || !error->start)
    return NULL;

  unsigned long nameLen = strlen(error->name);
  unsigned long detailsLen = strlen(error->details);
  unsigned long fileLen = strlen(error->filename);

  char lineBuf[32];
  snprintf(lineBuf, sizeof(lineBuf), "%lu", error->start->line + 1);
  unsigned long lineLen = strlen(lineBuf);

  char colBuf[32];
  snprintf(colBuf, sizeof(colBuf), "%lu", error->start->column + 1);
  unsigned long colLen = strlen(colBuf);

  const char *prefix1 = ": ";
  const char *prefix2 = "\nFile ";
  const char *prefix3 = ", line ";
  const char *prefix4 = ", column ";

  unsigned long total = nameLen + strlen(prefix1) + detailsLen + strlen(prefix2) + fileLen + strlen(prefix3) + lineLen + strlen(prefix4) + colLen + 1;

  char *s = malloc(total);
  if (!s) return NULL;

  char *p = s;

  memcpy(p, error->name, nameLen);
  p += nameLen;

  memcpy(p, prefix1, strlen(prefix1));
  p += strlen(prefix1);

  memcpy(p, error->details, detailsLen);
  p += detailsLen;

  memcpy(p, prefix2, strlen(prefix2));
  p += strlen(prefix2);

  memcpy(p, error->filename, fileLen);
  p += fileLen;

  memcpy(p, prefix3, strlen(prefix3));
  p += strlen(prefix3);

  memcpy(p, lineBuf, lineLen);
  p += lineLen;

  memcpy(p, prefix4, strlen(prefix4));
  p += strlen(prefix4);

  memcpy(p, colBuf, colLen);
  p += colLen;

  *p = '\0';

  return s;
}
