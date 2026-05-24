#include "../include/lexer.h"
#include "../include/error.h"
#include "../include/token.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

#include <stdio.h>

#define _is_letter(c) (((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
#define _is_digit(c) (c >= '0' && c <= '9')
#define _is_alnum(c) (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')

#define UNLIKELY(x) __builtin_expect(!!(x), 0)

void advanceLexer(Lexer *lexer) {
  if (!lexer) return;

  advancePosition(&lexer->pos, lexer->currChar);

  if (lexer->pos.index < lexer->textLen) {
    lexer->currChar = lexer->text[lexer->pos.index];
  } else {
    lexer->currChar = '\0';
  }
}

Lexer* initLexer(char *filename, char *text) {
  if (!text) return NULL;

  Lexer* lexer = malloc(sizeof(Lexer));

  if (!lexer) return NULL;

  lexer->text = text;

  if (!lexer->text) {
    free(lexer);
    return NULL;
  }

  lexer->filename = filename;
  lexer->textLen = strlen(text);
  lexer->currChar = 0;
  lexer->pos = (Position){-1, 0, -1, lexer->filename, lexer->text};

  advanceLexer(lexer);

  return lexer;
}

bool _resizeTokensList(Token ***tokens, unsigned long *capacity) {
  unsigned long newCap = (*capacity) * 2;

  Token **tmp = realloc(*tokens, newCap * sizeof(Token*));
  if (!tmp) return false;

  *tokens = tmp;
  *capacity = newCap;

  return true;
}

static TokType keywordType(const char *s) {
  switch (s[0]) {
    case 'A':
      if (s[1] == 'N' && s[2] == 'D' && s[3] == '\0')
        return TOK_AND;
      break;

    case 'E':
      if (s[1] == 'L' && s[2] == 'I' && s[3] == 'F' && s[4] == '\0')
        return TOK_ELIF;

      if (s[1] == 'N' && s[2] == 'D' && s[3] == '\0')
        return TOK_END;

      if (s[1] == 'L' && s[2] == 'S' && s[3] == 'E' && s[4] == '\0')
        return TOK_ELSE;
      break;

    case 'I':
      if (s[1] == 'F' && s[2] == '\0')
        return TOK_IF;
      break;

    case 'O':
      if (s[1] == 'R' && s[2] == '\0')
        return TOK_OR;
      break;

    case 'T':
      if (s[1] == 'H' && s[2] == 'E' && s[3] == 'N' && s[4] == '\0')
        return TOK_THEN;
      break;

    case 'V':
      if (s[1] == 'A' && s[2] == 'R' && s[3] == '\0')
        return TOK_VAR;
      break;

    case 'W':
      if (s[1] == 'H' && s[2] == 'I' && s[3] == 'L' && s[4] == 'E' && s[5] == '\0')
        return TOK_WHILE;
      break;
  }

  return TOK_IDENTIFIER;
}

Token* makeNumberTokenLexer(Lexer* lexer, Error** error) {
  if (!lexer) return NULL;

  unsigned long size = 0;
  unsigned long capacity = 32;

  char *numStr = malloc(capacity);

  if (!numStr) return NULL;

  unsigned long dotCount = 0;

  Position start = lexer->pos;

  while (lexer->currChar != 0 && (_is_digit(lexer->currChar) || lexer->currChar == '.')) {
    if (size >= capacity) {
      capacity *= 2;

      void* tmp = realloc(numStr, capacity);

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
      if (*error == NULL) *error = initLexerError(start, lexer->pos, lexer->filename, "Invalid numeral literal");
      return NULL;
    }

    // overflow / underflow
    if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
      free(numStr);
      *error = initSemanticError(start, lexer->pos, lexer->filename, "Number out of range");
      return NULL;
    }

    // trailing garbage
    if (*end != '\0') {
      free(numStr);
      *error = initLexerError(start, lexer->pos, lexer->filename, "Trailing characters after number");
      return NULL;
    }

    Token* token = initToken(TOK_INT, &value, false, start, lexer->pos);

    if (!token) {
      free(numStr);
      return NULL;
    }

    free(numStr);
    return token;
  }

  char *end;
  errno = 0;

  double value = strtod(numStr, &end);

  // No digits found
  if (end == numStr) {
    free(numStr);
    *error = initLexerError(start, lexer->pos, lexer->filename, "Invalid numeric literal");
    return NULL;
  }

  // overflow / underflow
  if (errno == ERANGE) {
    free(numStr);
    *error = initSemanticError(start, lexer->pos, lexer->filename, "Number out of range");
    return NULL;
  }

  // trailing garbage
  if (*end != '\0') {
    free(numStr);
    *error = initLexerError(start, lexer->pos, lexer->filename, "Trailing characters after number");
    return NULL;
  }

  Token* token = initToken(TOK_FLOAT, &value, false, start, lexer->pos);

  if (!token) {
    free(numStr);
    return NULL;
  }

  free(numStr);
  return token;
}

Token* makeIdentifierLexer(Lexer *lexer) {
  unsigned long size = 0;
  unsigned long capacity = 32;

  char *idStr = malloc(capacity);

  if (!idStr) return NULL;

  Position start = lexer->pos;

  while (lexer->currChar && (_is_alnum(lexer->currChar) || lexer->currChar == '_')) {
    if (size >= capacity) {
      capacity *= 2;

      void* tmp = realloc(idStr, capacity);

      if (!tmp) {
        free(idStr);
        return NULL;
      }

      idStr = tmp;
    }

    idStr[size++] = lexer->currChar;
    advanceLexer(lexer);
  }

  idStr[size] = '\0';
  TokType type = keywordType(idStr);

  if (type == TOK_IDENTIFIER) {
    return initToken(type, idStr, true, start, lexer->pos);
  }

  free(idStr);
  return initToken(type, NULL, false, start, lexer->pos);
}

Token* makeStringLexer(Lexer* lexer, Error** error) {
  Position start = lexer->pos;

  advanceLexer(lexer); // Skip opening quote

  char *buffer = malloc(1024); // fixed length for now
  unsigned long len = 0;

  while (lexer->currChar && lexer->currChar != '"') {
    if (lexer->currChar == '\\') {
        advanceLexer(lexer);

        switch (lexer->currChar) {
            case 'n': buffer[len++] = '\n'; break;
            case 't': buffer[len++] = '\t'; break;
            case '"': buffer[len++] = '"'; break;
            case '\\': buffer[len++] = '\\'; break;
            default: buffer[len++] = lexer->currChar; break;
        }
    } else {
        buffer[len++] = lexer->currChar;
    }

    advanceLexer(lexer);
  }

  if (!lexer->currChar) {
    if (*error == NULL) *error = initSyntaxError(start, lexer->pos, lexer->filename, "Unterminated string");
    return NULL;
  }

  buffer[len] = '\0';

  advanceLexer(lexer); // skip closing quote

  return initToken(TOK_STRING, buffer, true, start, lexer->pos);
}

bool _generateToken(Lexer *lexer, Token*** tokens, unsigned long *size, unsigned long *capacity, TokType tokenType) {
  if (UNLIKELY(*size >= *capacity)) {
    if (!_resizeTokensList(tokens, capacity)) {
      freeTokens(*tokens, *size);
      return false;
    }
  }

  Position start = lexer->pos;
  Position end = lexer->pos;
  advancePosition(&end, lexer->currChar);

  Token *token = initToken(tokenType, NULL, false, start, end);

  if (!token) {
    freeTokens(*tokens, *size);
    return false;
  }

  (*tokens)[*size] = token;
  (*size)++;

  advanceLexer(lexer);

  return true;
}

bool _appendToken(Token* token, Token*** tokens, unsigned long *size, unsigned long *capacity) {
  if (!token) {
    freeTokens(*tokens, *size);
    return false;
  }

  if (*size >= *capacity) {
    if (!_resizeTokensList(tokens, capacity)) {
      freeTokens(*tokens, *size);
      return false;
    }
  }

  (*tokens)[(*size)++] = token;
  return true;
}

Token* makeNotEqualsToken(Lexer* lexer, Error** error) {
  Position start = lexer->pos;
  advanceLexer(lexer);

  if (!lexer->currChar) {
    if (*error == NULL) *error = initSyntaxError(start, lexer->pos, start.filename, "Expected '=' symbol after '!'");
    return NULL;
  }

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_NE, NULL, false, start, lexer->pos);
  }

  if (*error == NULL) *error = initSyntaxError(start, lexer->pos, start.filename, "Expected '=' symbol after '!'");
  return NULL;
}

Token* makeEqualsToken(Lexer* lexer) {
  Position start = lexer->pos;

  advanceLexer(lexer);

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_EE, NULL, false, start, lexer->pos);
  }

  return initToken(TOK_EQ, NULL, false, start, lexer->pos);
}

Token* makeLessThanToken(Lexer* lexer) {
  Position start = lexer->pos;

  advanceLexer(lexer);

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_LTE, NULL, false, start, lexer->pos);
  }

  return initToken(TOK_LT, NULL, false, start, lexer->pos);
}

Token* makeGreaterThanToken(Lexer* lexer) {
  Position start = lexer->pos;

  advanceLexer(lexer);

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_GTE, NULL, false, start, lexer->pos);
  }

  return initToken(TOK_GT, NULL, false, start, lexer->pos);
}

Token** makeTokensLexer(Lexer *lexer, Error **error, unsigned long *outSize) {
  unsigned long size = 0;
  unsigned long capacity = 1024;

  Token** tokens = malloc(sizeof(Token*) * capacity);
  if (!tokens) return NULL;

  while (lexer->currChar != 0) {
    if (lexer->currChar == ' ' || lexer->currChar == '\t' || lexer->currChar == '\n') {
      advanceLexer(lexer);
      continue;
    }

    if (_is_digit(lexer->currChar)) {
      if (!_appendToken(makeNumberTokenLexer(lexer, error), &tokens, &size, &capacity)) return NULL;
      continue;
    }

    if (_is_letter(lexer->currChar)) {
      if (!_appendToken(makeIdentifierLexer(lexer), &tokens, &size, &capacity)) return NULL;
      continue;
    }

    if (lexer->currChar == '"') {
      if (!_appendToken(makeStringLexer(lexer, error), &tokens, &size, &capacity)) return NULL;
      continue;
    }

    if (lexer->currChar == '!') {
      if (!_appendToken(makeNotEqualsToken(lexer, error), &tokens, &size, &capacity)) return NULL;
      continue;
    }

    if (lexer->currChar == '=') {
      if (!_appendToken(makeEqualsToken(lexer), &tokens, &size, &capacity)) return NULL;
      continue;
    }

    if (lexer->currChar == '<') {
      if (!_appendToken(makeLessThanToken(lexer), &tokens, &size, &capacity)) return NULL;
      continue;
    }

    if (lexer->currChar == '>') {
      if (!_appendToken(makeGreaterThanToken(lexer), &tokens, &size, &capacity)) return NULL;
      continue;
    }

    if (lexer->currChar == '+') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_PLUS)) return NULL;
      continue;
    }

    if (lexer->currChar == '-') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_MINUS)) return NULL;
      continue;
    }

    if (lexer->currChar == '[') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_LBRACK)) return NULL;
      continue;
    }

    if (lexer->currChar == ']') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_RBRACK)) return NULL;
      continue;
    }

    if (lexer->currChar == ',') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_COMMA)) return NULL;
      continue;
    }

    if (lexer->currChar == '*') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_MUL)) return NULL;
      continue;
    }

    if (lexer->currChar == '/') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_DIV)) return NULL;
      continue;
    }

    if (lexer->currChar == '^') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_POW)) return NULL;
      continue;
    }

    if (lexer->currChar == '(') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_LPAREN)) return NULL;
      continue;
    }

    if (lexer->currChar == ')') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_RPAREN)) return NULL;
      continue;
    }

    char details[4] = {'\'', lexer->currChar, '\'', '\0'};
    Position start = lexer->pos;
    advanceLexer(lexer);
    Position end = lexer->pos;

    if (error) {
      *error = initIllegalCharError(start, end, lexer->filename, details);
    }

    freeTokens(tokens, size);
    return NULL;
  }

  tokens[size] = NULL;
  *outSize = size;
  return tokens;
}

void freeLexer(Lexer *lexer) {
  if (!lexer) return;
  free(lexer);
}
