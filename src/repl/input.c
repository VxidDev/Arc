#include "../../include/repl/input.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

struct termios origTermios;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &origTermios);
  
  static int atexitRegistered = 0;

  if (!atexitRegistered) {
    atexit(disableRawMode);
    atexitRegistered = 1;
  }

  struct termios raw = origTermios;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char *input(const char* prompt) {
  if (prompt) printf("%s", prompt);

  size_t capacity = 4096;
  size_t len = 0;

  char *buffer = malloc(capacity);

  if (!buffer) {
    return NULL;
  }

  size_t cursor = 0;

  enableRawMode();

  while (true) {
    int c = getchar();

    if (c == EOF) {
      break;
    }

    // Handle Backspace (127 or 8 depending on terminal)
    else if (c == 127 || c == 8) {
      if (cursor > 0) {
        // Shift string left from cursor position
        memmove(&buffer[cursor - 1], &buffer[cursor], len - cursor + 1);
        len--;
        cursor--;

        buffer[len] = '\0';
            
        // Move cursor back, print the rest of the line, clear trailing char, restore cursor
        printf("\b%s \b", &buffer[cursor]);
        for (size_t i = 0; i < len - cursor; i++) printf("\b");
        fflush(stdout);
      }
    } else if (c == '\n' || c == '\r') {
      buffer[len] = '\0';
      printf("\n");
      
      disableRawMode();

      return buffer;
    } else if (c == 27) { // 27 is the Escape character
      int next1 = getchar();
      int next2 = getchar();

      if (next1 == '[') {
        if (next2 == 'D') { // Left Arrow sequence: ESC [ D
          if (cursor > 0) {
            cursor--;
            printf("\033[1D"); // ANSI escape code to move cursor left 1 space
            fflush(stdout);
          }
        } else if (next2 == 'C') { // Right Arrow sequence: ESC [ C
          if (cursor < len) {
            cursor++;
            printf("\033[1C"); // ANSI escape code to move cursor right 1 space
            fflush(stdout);
          }
        }
      }
    } else if (c >= 32 && c <= 126) {
      if (len + 2 > capacity) {
        capacity *= 2;
        void* tmp = realloc(buffer, capacity);

        if (!tmp) {
          return NULL;
        }

        buffer = tmp;
      }

      // Insert character at current cursor position
      memmove(&buffer[cursor + 1], &buffer[cursor], len - cursor + 1);
      buffer[cursor] = c;
      len++;

      buffer[len] = '\0';
      
      // Print inserted character and the rest of the string shifted right
      printf("%s", &buffer[cursor]);
      cursor++;
      
      // Move terminal cursor back to its logical position
      for (size_t i = 0; i < len - cursor; i++) {
        printf("\b");
      }

      fflush(stdout);
    }
  }
  
  disableRawMode();
  return buffer;
}
