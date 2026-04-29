#ifndef ERROR_H
#define ERROR_H

#include "position.h"

typedef struct Error {
  char *name, *details, *filename;
  Position *start, *end;
} Error;

Error* initError(Position* start, Position* end, char *filename, char *name, char *details);
Error* initIllegalCharError(Position* start, Position* end, char *filename, char *details);
Error* initSyntaxError(Position* start, Position* end, char *filename, char *details);
Error* initValueError(Position* start, Position* end, char *filename, char *details);
Error* initLexerError(Position* start, Position* end, char *filename, char *details);
Error* initSemanticError(Position* start, Position* end, char *filename, char *details);

char *errorAsString(const Error *error);

void freeError(Error* err);

#endif // ERROR_H 
