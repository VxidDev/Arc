#ifndef ARC_RANDOM_H
#define ARC_RANDOM_H

#include "../../include/object.h"

void pcg32srandom(uint64_t initstate, uint64_t initseq);
Object* builtIn_randint(Object** args, size_t argCount);

#endif // ARC_RANDOM_H
