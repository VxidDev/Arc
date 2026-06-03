#include "../../include/repl/readfile.h"
#include "../../include/ansi-colors.h"
#include "../../include/repl/repl.h"

#include <stdlib.h>
#include <stdio.h>

#include <stdbool.h>
#include <string.h>

static inline bool isValidExtension(const char *filename) {
  if (!filename) return false;

  const char *dot = strrchr(filename, '.');

  // no dot or dot is first character
  if (!dot || dot == filename) return false;

  return strcmp(dot, ".arc") == 0;
}

char *readFile(const char *filename) {
  if (!isValidExtension(filename)) {
    printf("%sArc: %sInvalid file extension: expected \".arc\"%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
    return NULL;
  }

  FILE* file = fopen(filename, "rb");

  if (!file) {
    printf("%sArc: %sFailed to open file: \"%s\"%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), filename, COLOR(ANSI_RESET));
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);

  if (size < 0) {
    printf("%sArc: %sFailed to open file: \"%s\"%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), filename, COLOR(ANSI_RESET)); 
    fclose(file);
    return NULL;
  }

  rewind(file);

  char *buf = malloc(size + 1);

  if (!buf) {
    fclose(file);
    printf("%sArc: %sFailed to read file%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
    return NULL;
  } 

  size_t read = fread(buf, 1, size, file);

  if (read != (size_t)size) {
    printf("%sArc: %sFailed to read file%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET)); 
    fclose(file);
    return NULL;
  }

  buf[size] = '\0';

  fclose(file);

  return buf;
}

