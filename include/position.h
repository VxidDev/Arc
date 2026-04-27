#ifndef POSITION_H
#define POSITION_H

typedef struct Position {
  unsigned long index, line, column;
  char *filename, *filetext;
} Position;

Position *initPosition(unsigned long index, unsigned long line, unsigned long column, char *filename, char *filetext);

Position* advancePosition(Position *pos, char c);
Position* copyPosition(const Position *pos);

void freePosition(Position *pos);

#endif // POSITION_H
