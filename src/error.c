#include "../include/utils.h"
#include "../include/error.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void freeError(Error* err) {
  if (!err) return;

  free(err);
}

Error* initError(Position start, Position end, char *name, char *filename, char *details, char *filetext) {
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
    free(error);
    return NULL;
  }
  
  error->filename = stringDup(filename);
  error->start = start;
  error->end = end;
  error->filetext = stringDup(filetext);
  return error;
}

Error *initIllegalCharError(Position start, Position end, char *filename, char *details, char *filetext) {
  return initError(start, end, "Illegal Character", filename, details, filetext);
}

Error *initSyntaxError(Position start, Position end, char *filename, char* details, char *filetext) {
  return initError(start, end, "Syntax Error", filename, details, filetext);
}

Error *initValueError(Position start, Position end, char *filename, char* details, char *filetext) {
  return initError(start, end, "Value Error", filename, details, filetext);
}

Error *initLexerError(Position start, Position end, char *filename, char* details, char *filetext) {
  return initError(start, end, "Lexer Error", filename, details, filetext);
}

Error *initSemanticError(Position start, Position end, char *filename, char* details, char *filetext) {
  return initError(start, end, "Semantic Error", filename, details, filetext); 
}

Error *initNameError(Position start, Position end, char *filename, char* details, char *filetext) {
  return initError(start, end, "Name Error", filename, details, filetext);
}

Error *initTypeError(Position start, Position end, char *filename, char* details, char *filetext) {
  return initError(start, end, "Type Error", filename, details, filetext);
}

Error *initIndexError(Position start, Position end, char *filename, char* details, char *filetext) {
  return initError(start, end, "Index Error", filename, details, filetext);
}

Error *initRuntimeError(Position start, Position end, char *filename, char* details, char *filetext) {
  return initError(start, end, "Runtime Error", filename, details, filetext);
}

char* errorAsString(const Error* error) {
  if (!error || !error->name || !error->details || !error->filename || !error->filetext)
    return NULL;
  
  if (!error->filetext || !*error->filetext) {
    int needed = snprintf(NULL, 0, "%s: %s\nFile %s, line %lu, column %lu\n",
      error->name, error->details, error->filename,
      error->start.line + 1, error->start.column + 1);

    char *out = malloc(needed + 1);
    if (!out) return NULL;

    snprintf(out, needed + 1, "%s: %s\nFile %s, line %lu, column %lu\n",
      error->name, error->details, error->filename,
      error->start.line + 1, error->start.column + 1);

    return out;
  }

  const char *text = error->filetext;
  unsigned long startIdx = error->start.index;
  size_t textLen = strlen(text); 
  
  if (startIdx >= textLen) {
    int needed = snprintf(NULL, 0, "%s: %s\nFile %s, line %lu, column %lu\n",
      error->name, error->details, error->filename,
      error->start.line + 1, error->start.column + 1);

    char *out = malloc(needed + 1);
    if (!out) return NULL;

    snprintf(out, needed + 1, "%s: %s\nFile %s, line %lu, column %lu\n",
      error->name, error->details, error->filename,
      error->start.line + 1, error->start.column + 1);

    return out;
  }

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

  unsigned long startCol = error->start.column;
  unsigned long endCol = error->end.column;

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
    error->start.line + 1,
    error->start.column + 1,
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
    error->start.line + 1,
    error->start.column + 1,
    lineStr,
    underline
  );

  free(lineStr);
  free(underline);

  return result;
}
