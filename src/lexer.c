#include "../include/lexer.h"
#include "../include/utils.h"
#include "../include/error.h"
#include "../include/token.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

#include <stdio.h>

void advanceLexer(Lexer *lexer) {
  if (!lexer) return;
  
  lexer->currChar = (++lexer->pos->index < lexer->textLen) ? lexer->text[lexer->pos->index] : '\0';
}

Lexer* initLexer(char *filename, char *text) {
  if (!text) return NULL;

  Lexer* lexer = malloc(sizeof(Lexer));

  if (!lexer) return NULL;

  lexer->text = stringDup(text);

  if (!lexer->text) {
    free(lexer);
    return NULL;
  }

  lexer->filename = stringDup(filename);

  if (!lexer->filename) {
    free(lexer->text);
    free(lexer);
    return NULL;
  }
  
  lexer->textLen = strlen(text);
  lexer->currChar = 0;

  Position* pos = initPosition(-1, 0, -1, lexer->filename, lexer->text);

  if (!pos) {
    free(lexer->text);
    free(lexer->filename);
    free(lexer);
    return NULL;
  }

  lexer->pos = pos;

  advanceLexer(lexer);

  return lexer;
}

void _freeTokens(Token** tokens, unsigned long size) {
  if (!tokens) return;

  for (unsigned long i = 0; i < size; i++) {
    freeToken(tokens[i]);
  }

  free(tokens);
}

bool _resizeTokensList(Token ***tokens, unsigned long *capacity, unsigned long size) {
  *capacity *= 2;

  Token **tmp = realloc(*tokens, (*capacity) * sizeof(Token*));

  if (!tmp) {
    _freeTokens(*tokens, size);
    return false;
  }

  *tokens = tmp;
  return true;
}

bool _is_digit(char c) {
  return (c >= '0' && c <= '9');
}

Token* makeNumberTokenLexer(Lexer* lexer) {
  if (!lexer) return NULL;

  unsigned long size = 0;
  unsigned long capacity = 32;

  char *numStr = malloc(capacity * sizeof(char));

  if (!numStr) return NULL;

  unsigned long dotCount = 0;

  while (lexer->currChar != 0 && (_is_digit(lexer->currChar) || lexer->currChar == '.')) {
    if (size >= capacity) {
      capacity *= 2;

      void* tmp = realloc(numStr, capacity * sizeof(char));

      if (!tmp) {
        free(numStr);
        return NULL;
      }

      numStr = tmp;
    }

    if (lexer->currChar == '.') {
      if (dotCount == 1) break;
      dotCount++;
    }

    numStr[size++] = lexer->currChar;  

    advanceLexer(lexer);
  }

  numStr[size] = '\0';

  if (dotCount == 0) {
    char *end;
    errno = 0;

    long value = strtol(numStr, &end, 10);

    // No digits found
    if (end == numStr) {
      free(numStr);
      return NULL;
    }

    // overflow / underflow 
    if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
      free(numStr);
      return NULL;
    }

    // trailing garbage
    if (*end != '\0') {
      free(numStr);
      return NULL;
    }

    long *val = malloc(sizeof(long));

    if (!val) {
      free(numStr);
      return NULL;
    }

    *val = value;

    Token* token = initToken(TOK_INT, val, true);

    if (!token) {
      free(val);
      free(numStr);
      return NULL;
    }
  
    free(numStr);
    return token;
  }

  char *end;
  errno = 0;

  double value = strtof(numStr, &end);

  // No digits found
  if (end == numStr) {
    free(numStr);
    return NULL;
  }

  // overflow / underflow 
  if (errno == ERANGE) {
    free(numStr);
    return NULL;
  }

  // trailing garbage
  if (*end != '\0') {
    free(numStr);
    return NULL;
  }

  double *val = malloc(sizeof(double));

  if (!val) {
    free(numStr);
    return NULL;
  }

  *val = value;

  Token* token = initToken(TOK_FLOAT, val, true);

  if (!token) {
    free(val);
    free(numStr);
    return NULL;
  }
  
  free(numStr);
  return token;
}

// Will get optimized / shortened later.
Token** makeTokensLexer(Lexer *lexer, Error **error, unsigned long *outSize) {
  unsigned long size = 0;
  unsigned long capacity = 16;

  Token** tokens = malloc(sizeof(Token*) * (capacity + 1));
  if (!tokens) return NULL;
  
  while (lexer->currChar != 0) {
    if (lexer->currChar == ' ' || lexer->currChar == '\t') {
      advanceLexer(lexer);
      continue;
    }

    if (_is_digit(lexer->currChar)) {
      Token *token = makeNumberTokenLexer(lexer);

      if (!token) {
        _freeTokens(tokens, size);
        return NULL;
      }

      if (size >= capacity) {
        if (!_resizeTokensList(&tokens, &capacity, size)) {
          _freeTokens(tokens, size);
          return NULL;
        }
      }

      tokens[size++] = token;

      continue;
    } 

    if (lexer->currChar == '+') {
      Token *token = initToken(TOK_PLUS, NULL, false);

      if (!token) {
        _freeTokens(tokens, size);
        return NULL;
      }
      
      if (size >= capacity) {
        if (!_resizeTokensList(&tokens, &capacity, size)) {
          _freeTokens(tokens, size);
          return NULL;
        }
      }

      tokens[size++] = token;
      advanceLexer(lexer);

      continue;
    }

    if (lexer->currChar == '-') {
      Token *token = initToken(TOK_MINUS, NULL, false);

      if (!token) {
        _freeTokens(tokens, size);
        return NULL;
      }
      
      if (size >= capacity) {
        if (!_resizeTokensList(&tokens, &capacity, size)) {
          _freeTokens(tokens, size);
          return NULL;
        }
      } 

      tokens[size++] = token;
      advanceLexer(lexer);

      continue;
    }

    if (lexer->currChar == '*') {
      Token *token = initToken(TOK_MUL, NULL, false);

      if (!token) {
        _freeTokens(tokens, size);
        return NULL;
      }
      
      if (size >= capacity) {
        if (!_resizeTokensList(&tokens, &capacity, size)) {
          _freeTokens(tokens, size);
          return NULL;
        }
      }

      tokens[size++] = token;
      advanceLexer(lexer);

      continue;
    }

    if (lexer->currChar == '/') {
      Token *token = initToken(TOK_DIV, NULL, false);

      if (!token) {
        _freeTokens(tokens, size);
        return NULL;
      }
      
      if (size >= capacity) {
        if (!_resizeTokensList(&tokens, &capacity, size)) {
          _freeTokens(tokens, size);
          return NULL;
        }
      }

      tokens[size++] = token;
      advanceLexer(lexer);

      continue;
    }

    if (lexer->currChar == '(') {
      Token *token = initToken(TOK_LPAREN, NULL, false);

      if (!token) {
        _freeTokens(tokens, size);
        return NULL;
      }
      
      if (size >= capacity) {
        if (!_resizeTokensList(&tokens, &capacity, size)) {
          _freeTokens(tokens, size);
          return NULL;
        }
      }

      tokens[size++] = token;
      advanceLexer(lexer);

      continue;
    }

    if (lexer->currChar == ')') {
      Token *token = initToken(TOK_RPAREN, NULL, false);

      if (!token) {
        _freeTokens(tokens, size);
        return NULL;
      }
      
      if (size >= capacity) {
        if (!_resizeTokensList(&tokens, &capacity, size)) {
          _freeTokens(tokens, size);
          return NULL;
        }
      }

      tokens[size++] = token;
      advanceLexer(lexer);

      continue;
    }

    char details[4] = {'\'', lexer->currChar, '\'', '\0'};
    Position* start = copyPosition(lexer->pos);

    advanceLexer(lexer);

    Position* end = copyPosition(lexer->pos);
     
    if (error) {
      *error = initIllegalCharError(start, end, lexer->filename, details);
    }

    _freeTokens(tokens, size);
    return NULL;
  }
  
  tokens[size] = NULL;
  *outSize = size;
  return tokens;
}

void freeLexer(Lexer *lexer) {
  if (!lexer) return;

  if (lexer->text) free(lexer->text);
  if (lexer->filename) free(lexer->filename);
  
  if (lexer->pos) freePosition(lexer->pos); 

  free(lexer);
}
