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

Error *initNameError(Position* start, Position *end, char *filename, char* details) {
  return initError(start, end, "Name Error", filename, details);
}

char* errorAsString(const Error* error) {
  if (!error || !error->name || !error->details || !error->filename || !error->start || !error->end)
    return NULL;

  const char *text = error->start->filetext;
  unsigned long startIdx = error->start->index;

  unsigned long lineStart = startIdx;
  while (lineStart > 0 && text[lineStart - 1] != '\n')
    lineStart--;

  unsigned long lineEnd = startIdx;
  while (text[lineEnd] && text[lineEnd] != '\n')
    lineEnd++;

  unsigned long lineLen = lineEnd - lineStart;

  char *lineStr = malloc(lineLen + 1);
  if (!lineStr) return NULL;

  memcpy(lineStr, text + lineStart, lineLen);
  lineStr[lineLen] = '\0';

  unsigned long startCol = error->start->column;
  unsigned long endCol = error->end->column;

  if (endCol < startCol) endCol = startCol;

  unsigned long underlineLen = endCol;
  char *underline = malloc(underlineLen + 1);
  if (!underline) {
    free(lineStr);
    return NULL;
  }

  for (unsigned long i = 0; i < underlineLen; i++) {
    underline[i] = (i >= startCol && i <= endCol) ? '^' : ' ';
  }

  underline[underlineLen] = '\0';

  int needed = snprintf(
    NULL, 0,
    "%s: %s\n"
    "File %s, line %lu, column %lu\n\n"
    "%s\n"
    "%s\n",
    error->name,
    error->details,
    error->filename,
    error->start->line + 1,
    error->start->column + 1,
    lineStr,
    underline
  );

  char *result = malloc(needed + 1);
  if (!result) {
    free(lineStr);
    free(underline);
    return NULL;
  }

  snprintf(
    result, needed + 1,
    "%s: %s\n"
    "File %s, line %lu, column %lu\n\n"
    "%s\n"
    "%s\n",
    error->name,
    error->details,
    error->filename,
    error->start->line + 1,
    error->start->column + 1,
    lineStr,
    underline
  );

  free(lineStr);
  free(underline);

  return result;
}
