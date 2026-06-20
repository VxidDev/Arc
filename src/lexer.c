#include "../include/lexer.h"
#include "../include/error.h"
#include "../include/token.h"

#include "../include/memarena.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

#include <ctype.h>

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

  Lexer* lexer = arenaAlloc(parseArena, sizeof(Lexer));

  if (!lexer) return NULL;

  lexer->text = text;

  if (!lexer->text) {
    free(lexer);
    return NULL;
  }

  lexer->filename = filename;
  lexer->textLen = strlen(text);
  lexer->currChar = 0;
  lexer->pos = (Position){-1, 0, -1};

  advanceLexer(lexer);

  return lexer;
}

bool _resizeTokensList(Token **tokens, unsigned long *capacity) {
  unsigned long newCap = (*capacity) * 2;

  Token *tmp = arenaRealloc(parseArena, *tokens, *capacity * sizeof(Token), newCap * sizeof(Token));
  if (!tmp) return false;

  *tokens = tmp;
  *capacity = newCap;

  return true;
}

static TokType keywordType(const char *s) {
  char c = toupper(s[0]);

  switch (c) {
    case 'A':
      if (strcasecmp("AND", s) == 0)
        return TOK_AND;

      break;

    case 'B':
      if (strcasecmp("BREAK", s) == 0)
        return TOK_BREAK; 

      break;

    case 'E':
      if (strcasecmp("ELIF", s) == 0)
        return TOK_ELIF;

      if (strcasecmp("END", s) == 0)
        return TOK_END;

      if (strcasecmp("ELSE", s) == 0)
        return TOK_ELSE;
      break; 

    case 'I':
      if ((s[1] == 'F' || s[1] == 'f') && s[2] == '\0')
        return TOK_IF;
      
      if ((s[1] == 'N' || s[1] == 'n') && s[2] == '\0')
        return TOK_IN;

      if (strcasecmp("IMPORT", s) == 0)
        return TOK_IMPORT;

      break;

    case 'O':
      if ((s[1] == 'R' || s[1] == 'r') && s[2] == '\0')
        return TOK_OR;
      break;

    case 'T':
      if (strcasecmp("THEN", s) == 0)
        return TOK_THEN;

      if (strcasecmp("TRY", s) == 0)
        return TOK_TRY;

      break;

    case 'V':
      if (strcasecmp("VAR", s) == 0)
        return TOK_VAR;

      break;

    case 'W':
      if (strcasecmp("WHILE", s) == 0)
        return TOK_WHILE;

      break;

    case 'F':
      if (strcasecmp("FOR", s) == 0)
        return TOK_FOR;
      
      if ((s[1] == 'N' || s[1] == 'n') && s[2] == '\0')
        return TOK_FN;
      break;

    case 'R':
      if (strcasecmp("RETURN", s) == 0)
        return TOK_RETURN;

      break;

    case 'C':
      if (strcasecmp("CATCH", s) == 0)
        return TOK_CATCH;

      if (strcasecmp("CONTINUE", s) == 0)
        return TOK_CONTINUE;

      if (strcasecmp("CLASS", s) == 0)
        return TOK_CLASS;

      break;

    case 'N':
      if (strcasecmp("NOT", s) == 0)
        return TOK_NOT;

      if (strcasecmp("NULL", s) == 0)
        return TOK_NULL;

      break;
  }

  return TOK_IDENTIFIER;
}

Token makeNumberTokenLexer(Lexer* lexer, Error** error) {
  if (!lexer) return (Token){.type = TOK_INVALID};

  size_t size = 0;
  size_t capacity = 32;

  char *numStr = arenaAlloc(stringArena, capacity);

  if (!numStr) return (Token){.type = TOK_INVALID};

  size_t dotCount = 0;

  Position start = lexer->pos;

  while (lexer->currChar != 0 && (_is_digit(lexer->currChar) || lexer->currChar == '.')) {
    if (size >= capacity) {
      size_t oldcap = capacity;
      capacity *= 2;

      void* tmp = arenaRealloc(stringArena, numStr, oldcap, capacity);

      if (!tmp) {
        return (Token){.type = TOK_INVALID};
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

    int64_t value = (int64_t)strtoll(numStr, &end, 10);

    // No digits found
    if (end == numStr) {
      if (*error == NULL) *error = initLexerError(start, lexer->pos, lexer->filename, "Invalid numeral literal", lexer->text);
      return (Token){.type = TOK_INVALID};
    }

    // overflow / underflow
    if (errno == ERANGE || value > INT64_MAX || value < INT64_MIN) {
      *error = initSemanticError(start, lexer->pos, lexer->filename, "Number out of range", lexer->text);
      return (Token){.type = TOK_INVALID};
    }

    // trailing garbage
    if (*end != '\0') {
      *error = initLexerError(start, lexer->pos, lexer->filename, "Trailing characters after number", lexer->text);
      return (Token){.type = TOK_INVALID};
    }

    Token token = initToken(TOK_INT, &value, false, start, lexer->pos);

    return token;
  }

  char *end;
  errno = 0;

  double value = strtod(numStr, &end);

  // No digits found
  if (end == numStr) {
    *error = initLexerError(start, lexer->pos, lexer->filename, "Invalid numeric literal", lexer->text);
    return (Token){.type = TOK_INVALID};
  }

  // overflow / underflow
  if (errno == ERANGE) {
    *error = initSemanticError(start, lexer->pos, lexer->filename, "Number out of range", lexer->text);
    return (Token){.type = TOK_INVALID};
  }

  // trailing garbage
  if (*end != '\0') {
    *error = initLexerError(start, lexer->pos, lexer->filename, "Trailing characters after number", lexer->text);
    return (Token){.type = TOK_INVALID};
  }

  Token token = initToken(TOK_FLOAT, &value, false, start, lexer->pos);

  return token;
}

Token makeIdentifierLexer(Lexer *lexer) {
  unsigned long size = 0;
  unsigned long capacity = 32;

  char *idStr = arenaAlloc(stringArena, capacity);

  if (!idStr) return (Token){.type = TOK_INVALID};

  Position start = lexer->pos;

  while (lexer->currChar && (_is_alnum(lexer->currChar) || lexer->currChar == '_')) {
    if (size >= capacity) {
      size_t oldcap = capacity;
      capacity *= 2;

      void* tmp = arenaRealloc(stringArena, idStr, oldcap, capacity);

      if (!tmp) {
        return (Token){.type = TOK_INVALID};
      }

      idStr = tmp;
    }

    idStr[size++] = lexer->currChar;
    advanceLexer(lexer);
  }

  idStr[size] = '\0';

  if (strcmp(idStr, "true") == 0) {
    int64_t *val = arenaAlloc(stringArena, sizeof(int64_t));
    if (!val) return (Token){.type = TOK_INVALID};
    *val = 1;

    return initToken(TOK_INT, val, false, start, lexer->pos);
  }

  if (strcmp(idStr, "false") == 0) {
    int64_t *val = arenaAlloc(stringArena, sizeof(int64_t));
    if (!val) return (Token){.type = TOK_INVALID};
    *val = 0;

    return initToken(TOK_INT, val, false, start, lexer->pos);
  }

  TokType type = keywordType(idStr);

  if (type == TOK_IDENTIFIER) {
    return initToken(type, idStr, false, start, lexer->pos);
  }

  return initToken(type, NULL, false, start, lexer->pos);
}

Token makeStringLexer(Lexer* lexer, Error** error) {
  Position start = lexer->pos;

  advanceLexer(lexer); // Skip opening quote

  char *buffer = arenaAlloc(stringArena, 1024); // fixed length for now
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
    if (*error == NULL) *error = initSyntaxError(start, lexer->pos, lexer->filename, "Unterminated string", lexer->text);
    return (Token){.type = TOK_INVALID};
  }

  buffer[len] = '\0';

  advanceLexer(lexer); // skip closing quote

  return initToken(TOK_STRING, buffer, false, start, lexer->pos);
}

bool _generateToken(Lexer *lexer, Token** tokens, unsigned long *size, unsigned long *capacity, TokType tokenType) {
  if (UNLIKELY(*size >= *capacity)) {
    if (!_resizeTokensList(tokens, capacity)) {
      freeTokens(*tokens, *size);
      return false;
    }
  }

  Position start = lexer->pos;
  Position end = lexer->pos;
  advancePosition(&end, lexer->currChar);

  Token token = initToken(tokenType, NULL, false, start, end);

  if (token.type == TOK_INVALID) {
    freeTokens(*tokens, *size);
    return false;
  }

  (*tokens)[*size] = token;
  (*size)++;

  advanceLexer(lexer);

  return true;
}

bool _appendToken(Token token, Token** tokens, unsigned long *size, unsigned long *capacity) {
  if (token.type == TOK_INVALID) {
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

Token makeNotEqualsToken(Lexer* lexer, Error** error) {
  Position start = lexer->pos;
  advanceLexer(lexer);

  if (!lexer->currChar) {
    if (*error == NULL) *error = initSyntaxError(start, lexer->pos, lexer->filename, "Expected '=' symbol after '!'", lexer->text);
    return (Token){.type = TOK_INVALID};
  }

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_NE, NULL, false, start, lexer->pos);
  }

  if (*error == NULL) *error = initSyntaxError(start, lexer->pos, lexer->filename, "Expected '=' symbol after '!'", lexer->text);
  return (Token){.type = TOK_INVALID};
}

Token makeEqualsToken(Lexer* lexer) {
  Position start = lexer->pos;

  advanceLexer(lexer);

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_EE, NULL, false, start, lexer->pos);
  }

  return initToken(TOK_EQ, NULL, false, start, lexer->pos);
}

Token makeLessThanToken(Lexer* lexer) {
  Position start = lexer->pos;

  advanceLexer(lexer);

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_LTE, NULL, false, start, lexer->pos);
  }

  return initToken(TOK_LT, NULL, false, start, lexer->pos);
}

Token makeGreaterThanToken(Lexer* lexer) {
  Position start = lexer->pos;

  advanceLexer(lexer);

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_GTE, NULL, false, start, lexer->pos);
  }

  return initToken(TOK_GT, NULL, false, start, lexer->pos);
}

Token* makeTokensLexer(Lexer *lexer, Error **error, unsigned long *outSize) {
  unsigned long size = 0;
  unsigned long capacity = lexer->textLen / 2 + 64;

  Token* tokens = arenaAlloc(parseArena, sizeof(Token) * capacity);
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

    if (_is_letter(lexer->currChar) || lexer->currChar == '_') {
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

    if (lexer->currChar == '#') {
      while (lexer->currChar != '\n' && lexer->currChar != '\0') advanceLexer(lexer);
      continue;
    }

    if (lexer->currChar == '.') {
      if (!_generateToken(lexer, &tokens, &size, &capacity, TOK_DOT)) return NULL;
      continue;
    }

    char details[4] = {'\'', lexer->currChar, '\'', '\0'};
    Position start = lexer->pos;
    advanceLexer(lexer);
    Position end = lexer->pos;

    if (error) {
      *error = initIllegalCharError(start, end, lexer->filename, details, lexer->text);
    }

    freeTokens(tokens, size);
    return NULL;
  }
  tokens[size] = (Token){.type = TOK_EOF};
  *outSize = size;
  return tokens;
}

void freeLexer(Lexer *lexer) {
  if (!lexer) return;

  free(lexer);
}
