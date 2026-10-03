#ifndef POSITION_H
#define POSITION_H

#include <stdint.h>

typedef struct Position {
  uint32_t index, line, column;
} Position;

void advancePosition(Position *pos, char c);

#endif // POSITION_H
