#ifndef POSITION_H
#define POSITION_H

typedef struct Position {
  unsigned long index, line, column;
  char *filename, *filetext;
} Position;

void advancePosition(Position *pos, char c);

#endif // POSITION_H
