#include "../../include/builtIns/time.h"
#include "../../include/utils.h"

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

Object* builtIn_sleep(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  int64_t ms = ((Number*)args[0])->as.i;

  Sleep(ms);

  return (Object*)initInt(1);
}

#else

#include <time.h>
#include <unistd.h>

Object* builtIn_perf_counter(Object** args, size_t argCount) {
  (void)args;
  (void)argCount;

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);

  return (Object*)initFloat((double)ts.tv_sec + (double)ts.tv_nsec * 1e-9);
}

Object* builtIn_sleep(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  int64_t ms = ((Number*)args[0])->as.i;

  usleep(ms * 1000);

  return (Object*)initInt(1);
}

#endif
