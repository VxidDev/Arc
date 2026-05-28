#ifndef UTILS_H
#define UTILS_H

#include "token.h"
#include "object.h"

char *stringDup(const char *s);
char *tokToString(const TokType type);
char *typeofobj(const Object* obj);

#endif // UTILS_H 
