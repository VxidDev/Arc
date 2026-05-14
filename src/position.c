#include "../include/position.h"

void advancePosition(Position *pos, char c) {
  if (!pos) return;
  
  pos->index++;

  if (c == '\n') {
    pos->line++;
    pos->column = 0;
  } else {
    pos->column++;
  }
}
