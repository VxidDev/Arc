#include "../../include/repl/readfile.h"
#include "../../include/ansi-colors.h"
#include "../../include/repl/repl.h"

#include <stdlib.h>
#include <stdio.h>

#include <stdbool.h>
#include <string.h>

bool isValidExtension(const char *filename) {
  if (!filename) return false;

  const char *dot = strrchr(filename, '.');

  // no dot or dot is first character
  if (!dot || dot == filename) return false;

  return strcmp(dot, ".arc") == 0;
}

char *readFile(char *filename) {
  if (!isValidExtension(filename)) {
    printf("%sArc: %sInvalid file extension: expected \".arc\"%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
    return NULL;
  }

  FILE* file = fopen(filename, "rb");

  if (!file) {
    printf("%sArc: %sFailed to open file: \"%s\"%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), filename, COLOR(ANSI_RESET));
    return NULL;
  }

  size_t size = 0;
  size_t capacity = 128;

  char *buf = calloc(capacity, 1);

  if (!buf) {
    fclose(file);
    printf("%sArc: %sFailed to read file%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
    return NULL;
  }

  int c = 0;

  while ((c = getc(file)) != EOF) {
    if (size >= capacity - 1) {
      capacity *= 2;
      void *tmp = realloc(buf, sizeof(char) * capacity);

      if (!tmp) {
        free(buf);
        printf("%sArc: %sFailed to read file%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        return NULL;
      }

      buf = tmp;
    }

    buf[size++] = c;
  }

  buf[size] = '\0';

  fclose(file);

  return buf;
}

