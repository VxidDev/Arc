#ifndef ARC_TIME_H
#define ARC_TIME_H

#include "../object.h"

Object* builtIn_perf_counter(Object** args, size_t argCount);
Object* builtIn_sleep(Object** args, size_t argCount);

#endif // ARC_TIME_H
