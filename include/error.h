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
  Position *start, *end;
} Error;

Error* initError(Position* start, Position* end, char *filename, char *name, char *details);
Error* initIllegalCharError(Position* start, Position* end, char *filename, char *details);
Error* initSyntaxError(Position* start, Position* end, char *filename, char *details);
Error* initValueError(Position* start, Position* end, char *filename, char *details);
Error* initLexerError(Position* start, Position* end, char *filename, char *details);
Error* initSemanticError(Position* start, Position* end, char *filename, char *details);
Error* initNameError(Position* start, Position* end, char *filename, char *details);
Error* initTypeError(Position* start, Position* end, char *filename, char *details);

char *errorAsString(const Error *error);

void freeError(Error* err);

#endif // ERROR_H 
