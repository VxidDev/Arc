#include "../../include/object.h"
#include "../../include/utils.h"

#include <stdint.h>

typedef struct { 
  uint64_t state;
  uint64_t inc; 
} pcg32random_t;

static pcg32random_t rng = {0};

uint32_t pcg32random() {
  uint64_t oldstate = rng.state;
  rng.state = oldstate * 6364136223846793005ULL + rng.inc;
  uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
  uint32_t rot = (uint32_t)(oldstate >> 59u);
    
  return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void pcg32srandom(uint64_t initstate, uint64_t initseq) {
  rng.state = 0U;
  rng.inc = (initseq << 1u) | 1u;
  
  pcg32random(); 
  rng.state += initstate;
  pcg32random();
}

Object* builtIn_randint(Object** args, size_t argCount) {
  (void)argCount;

  Object* err = enforceType(args[0], OBJ_NUMBER_INT, 1);
  if (err) return err;

  int64_t end = ((Number*)args[0])->as.i;

  return (Object*)initInt(pcg32random() % end);
}
