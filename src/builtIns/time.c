#include "../../include/builtIns/time.h"

#ifdef _WIN32
#include <windows.h>

Object* builtIn_perf_counter(Object** args, size_t argCount) {
    (void)args;
    (void)argCount;

    static LARGE_INTEGER freq;
    static int initialized = 0;

    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }

    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);

    return (Object*)initFloat((double)count.QuadPart / (double)freq.QuadPart);
}

#else
#include <time.h>

Object* builtIn_perf_counter(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);

  return (Object*)initFloat((double)ts.tv_sec + (double)ts.tv_nsec * 1e-9);
}

#endif
