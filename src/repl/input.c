#include "../../include/repl/input.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

char* input(const char *prompt) {
  if (prompt) printf("%s", prompt);

  size_t size = 0;
  size_t capacity = 128;

  char *buf = calloc(capacity, sizeof(char));

  if (!buf) return NULL; 

  int c = 0; 

  while ((c = getchar()) != EOF && c != '\n') {
    if (size >= capacity - 1) {
      capacity *= 2;
      void *tmp = realloc(buf, sizeof(char) * capacity);

      if (!tmp) {
        free(buf);
        return NULL;
      }

      buf = tmp;
    }

    buf[size++] = c;
  }

  buf[size] = '\0';

  return buf; 
} 

