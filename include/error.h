#ifndef ERROR_H
#define ERROR_H

#include "position.h"

typedef enum ErrType {
  ERR_NONE,
  ERR_DIV_BY_ZERO,
  ERR_NULL,
  ERR_TYPE
} ErrType;

typedef struct Error {
  char *name, *details, *filename, *filetext;
  Position start, end;
} Error;

Error* initError(Position start, Position end, char *filename, char *name, char *details, char *filetext);
Error* initIllegalCharError(Position start, Position end, char *filename, char *details, char *filetext);
Error* initSyntaxError(Position start, Position end, char *filename, char *details, char *filetext);
Error* initValueError(Position start, Position end, char *filename, char *details, char *filetext);
Error* initLexerError(Position start, Position end, char *filename, char *details, char *filetext);
Error* initSemanticError(Position start, Position end, char *filename, char *details, char *filetext);
Error* initNameError(Position start, Position end, char *filename, char *details, char *filetext);
Error* initTypeError(Position start, Position end, char *filename, char *details, char *filetext);
Error* initRuntimeError(Position start, Position end, char *filename, char *details, char *filetext);
Error* initIndexError(Position start, Position end, char *filename, char *details, char *filetext);

char *errorAsString(const Error *error);

void freeError(Error* err);

#endif // ERROR_H 
