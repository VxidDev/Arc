#include "../include/position.h"
#include "../include/utils.h"

#include <stdlib.h>
#include <stddef.h>

void freePosition(Position *pos) {
  if (!pos) return;

  if (pos->filename) free(pos->filename);
  if (pos->filetext) free(pos->filetext);

  free(pos);
}

Position* advancePosition(Position *pos, char c) {
  if (!pos) return NULL;
  
  pos->index++;

  if (c == '\n') {
    pos->line++;
    pos->column = 0;
  } else {
    pos->column++;
  }

  return pos;
}

Position* initPosition(unsigned long index, unsigned long line, unsigned long column, char *filename, char *filetext) {
  Position *pos = malloc(sizeof(Position));

  if (!pos) return NULL;

  pos->index = index;
  pos->line = line;
  pos->column = column;

  pos->filename = stringDup(filename);

  if (!pos->filename) {
    free(pos);
    return NULL;
  }

  pos->filetext = stringDup(filetext);
  
  if (!pos->filetext) {
    free(pos->filename);
    free(pos);
    return NULL;
  }

  return pos;
}

Position* copyPosition(const Position *pos) {
  if (!pos) return NULL;

  Position *position = malloc(sizeof(Position));

  if (!position) return NULL;

  position->index = pos->index;
  position->column = pos->column;
  position->line = pos->line;
  position->filename = stringDup(pos->filename);

  if (!position->filename) {
    free(position);
    return NULL;
  }

  position->filetext = stringDup(pos->filetext);

  if (!position->filetext) {
    free(position->filename);
    free(position);
    return NULL;
  }

  return position;
}
