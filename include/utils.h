#ifndef UTILS_H
#define UTILS_H

#include "token.h"
#include "object.h"

char *stringDup(const char *s);
uint32_t hashStr(const char *str, size_t len);
char *tokToString(const TokType type);
char *typeofobj(const Object* obj);

void getDirectory(const char *path, char *out);

char *resolveImportPath(const char *currentFile, const char *importPath);

#endif // UTILS_H 
