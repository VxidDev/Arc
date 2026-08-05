#include "../include/lexer.h"
#include "../include/error.h"
#include "../include/token.h"

#include "../include/memarena.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#include <ctype.h>

#include <stdio.h>

#define _is_letter(c) (((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
#define _is_digit(c) (c >= '0' && c <= '9')
#define _is_alnum(c)  (_is_letter(c) || _is_digit(c))

#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define CASE_MASK 0xDFDFDFDFDFDFDFDFULL 

static inline uint64_t loadKeyword64(const char *s, size_t len) {
  uint64_t val = 0;
  memcpy(&val, s, len);
  return val & CASE_MASK;
}

#define STATIC_ASSERT_EXPR(cond) (sizeof(char[1 - 2 * !(cond)]) * 0)
#define KW(str) (STATIC_ASSERT_EXPR(sizeof(str) - 1 <= 8) + loadKeyword64(str, sizeof(str) - 1))

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
    return NULL;
  }

  lexer->filename = filename;
  lexer->textLen = strlen(text);
  lexer->currChar = 0;
  lexer->pos = (Position){-1, 0, -1};

  advanceLexer(lexer);

  return lexer;
}

static TokType keywordType(const char *s, const uint64_t len) {
  if (len > 8) return TOK_IDENTIFIER;
  
  const uint64_t word = loadKeyword64(s, len);

  switch (len) {
    case 2:
      if (word == KW("IF")) return TOK_IF;
      if (word == KW("FN")) return TOK_FN;
      if (word == KW("OR")) return TOK_OR;
      if (word == KW("IN")) return TOK_IN;

      break;

    case 3:
      if (word == KW("AND")) return TOK_AND;
      if (word == KW("END")) return TOK_END;
      if (word == KW("TRY")) return TOK_TRY;
      if (word == KW("VAR")) return TOK_VAR;
      if (word == KW("FOR")) return TOK_FOR;
      if (word == KW("NOT")) return TOK_NOT;

      break;

    case 4:
      if (word == KW("ELIF")) return TOK_ELIF;
      if (word == KW("ELSE")) return TOK_ELSE;
      if (word == KW("THEN")) return TOK_THEN;
      if (word == KW("NULL")) return TOK_NULL;
      if (word == KW("TRUE")) return TOK_TRUE;

      break;

    case 5:
      if (word == KW("BREAK")) return TOK_BREAK;
      if (word == KW("WHILE")) return TOK_WHILE;
      if (word == KW("CATCH")) return TOK_CATCH;
      if (word == KW("CLASS")) return TOK_CLASS;
      if (word == KW("FALSE")) return TOK_FALSE;

      break;
    
    case 6:
      if (word == KW("IMPORT")) return TOK_IMPORT;
      if (word == KW("RETURN")) return TOK_RETURN;      

      break;
    
    case 8:
      if (word == KW("CONTINUE")) return TOK_CONTINUE;
      
      break;
  }

  return TOK_IDENTIFIER;
}

static Token makeNumberTokenLexer(Lexer* lexer, Error** error) {
  if (!lexer) return (Token){.type = TOK_INVALID};
  
  Position start = lexer->pos;
  const char *startPtr = &lexer->text[lexer->pos.index];
  size_t len = 0;
  size_t dotCount = 0;
  
  while (lexer->currChar != '\0' && (_is_digit(lexer->currChar) || lexer->currChar == '.')) {
    if (lexer->currChar == '.') {
      if (dotCount == 1) break;
      dotCount++;
    }

    len++;
    advanceLexer(lexer);
  }

  char stackBuf[128];
  char *numStr = stackBuf;

  if (len >= sizeof(stackBuf)) {
    numStr = arenaAlloc(stringArena, len + 1);
    if (!numStr) return (Token){ .type = TOK_INVALID };
  }

  memcpy(numStr, startPtr, len);
  numStr[len] = '\0';

  char *end;
  errno = 0;
  
  if (dotCount == 0) {
    int64_t value = (int64_t)strtoll(numStr, &end, 10);

    // No digits found
    if (end == numStr) {
      if (*error == NULL) *error = initLexerError(start, lexer->pos, lexer->filename, "Invalid numeral literal", lexer->text);
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

    Token token = initToken(TOK_INT, &value, false, start, lexer->pos);

    return token;
  }

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

static Token makeIdentifierLexer(Lexer *lexer) {
  if (!lexer) return (Token){ .type = TOK_INVALID };

  Position start = lexer->pos;
  const char *startPtr = &lexer->text[lexer->pos.index];
  size_t len = 0;

  while (lexer->currChar && (_is_alnum(lexer->currChar) || lexer->currChar == '_')) {
    len++;
    advanceLexer(lexer);
  }

  TokType type = keywordType(startPtr, len);

  if (type != TOK_IDENTIFIER) {
    return initToken(type, NULL, false, start, lexer->pos);
  }

  char *idStr = arenaAlloc(stringArena, len + 1);
  if (!idStr) return (Token){ .type = TOK_INVALID };
  
  memcpy(idStr, startPtr, len);
  idStr[len] = '\0';

  return initToken(TOK_IDENTIFIER, idStr, false, start, lexer->pos);
}

Token makeStringLexer(Lexer* lexer, Error** error) {
  if (!lexer) return (Token){ .type = TOK_INVALID };

  Position start = lexer->pos;
  advanceLexer(lexer); // Skip opening quote
  
  char stackBuf[256];
  char *buffer = stackBuf;
  size_t capacity = sizeof(stackBuf);
  size_t len = 0;

  while (lexer->currChar != '\0' && lexer->currChar != '"') {
    if (len + 1 >= capacity) {
      size_t oldCap = capacity;
      capacity *= 2;

      if (buffer == stackBuf) {
        buffer = arenaAlloc(stringArena, capacity);
        if (!buffer) return (Token){ .type = TOK_INVALID };
        memcpy(buffer, stackBuf, len);
      } else {
        void *tmp = arenaRealloc(stringArena, buffer, oldCap, capacity);
        if (!tmp) return (Token){ .type = TOK_INVALID };
        buffer = tmp;
      }
    }

    if (lexer->currChar == '\\') {
      advanceLexer(lexer);

      switch (lexer->currChar) {
        case 'n': buffer[len++] = '\n'; break;
        case 't': buffer[len++] = '\t'; break;
        case 'r': buffer[len++] = '\r'; break;
        case '0': buffer[len++] = '\0'; break;
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
  
  char *finalStr;

  if (buffer == stackBuf) {
    finalStr = arenaAlloc(stringArena, len + 1);
    if (!finalStr) return (Token){ .type = TOK_INVALID };
    memcpy(finalStr, stackBuf, len + 1);
  } else {
    // trim block down to exact byte count
    finalStr = arenaRealloc(stringArena, buffer, capacity, len + 1);
  }

  return initToken(TOK_STRING, finalStr, false, start, lexer->pos);
}

static Token makeCharLexer(Lexer *lexer, Error** error) {
  Position start = lexer->pos;
  advanceLexer(lexer); // skip '\''
  
  int64_t value;

  if (lexer->currChar == '\\') {
    advanceLexer(lexer); // consume backslash

    switch (lexer->currChar) {
      case 'n': value = '\n'; break;
      case 't': value = '\t'; break;
      case 'r': value = '\r'; break;
      case '0': value = '\0'; break;
      case '\\': value = '\\'; break;
      case '\'': value = '\''; break;
      case '"': value = '"'; break;
      default:
        if (*error == NULL) *error = initSyntaxError(start, lexer->pos, lexer->filename, "Unknown escape sequence in char literal.", lexer->text);
        return (Token){.type = TOK_INVALID};
    }

    advanceLexer(lexer);
  } else if (lexer->currChar == '\'' || !lexer->currChar) {
    if (*error == NULL) *error = initSyntaxError(start, lexer->pos, lexer->filename, "Char literal must contain exactly one character.", lexer->text);
    return (Token){.type = TOK_INVALID};
  } else { 
    uint8_t raw = lexer->currChar;

    if (raw > 0x7F) {
      if (*error == NULL) *error = initSyntaxError(start, lexer->pos, lexer->filename, "Char literals only support single-byte (ASCII) characters.", lexer->text);
      return (Token){.type = TOK_INVALID};
    }

    value = (int64_t)raw;
    advanceLexer(lexer);
  }

  if (lexer->currChar != '\'') {
    if (*error == NULL) *error = initSyntaxError(start, lexer->pos, lexer->filename, "Char literal must contain exactly one character.", lexer->text);
    return (Token){.type = TOK_INVALID};
  }
  advanceLexer(lexer);
  
  return initToken(TOK_INT, &value, false, start, lexer->pos);
}

static Token makeNotEqualsToken(Lexer* lexer, Error** error) {
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

static Token makeEqualsToken(Lexer* lexer) {
  Position start = lexer->pos;

  advanceLexer(lexer);

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_EE, NULL, false, start, lexer->pos);
  }

  return initToken(TOK_EQ, NULL, false, start, lexer->pos);
}

static Token makeLessThanToken(Lexer* lexer) {
  Position start = lexer->pos;

  advanceLexer(lexer);

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_LTE, NULL, false, start, lexer->pos);
  }

  return initToken(TOK_LT, NULL, false, start, lexer->pos);
}

static Token makeGreaterThanToken(Lexer* lexer) {
  Position start = lexer->pos;

  advanceLexer(lexer);

  if (lexer->currChar == '=') {
    advanceLexer(lexer);
    return initToken(TOK_GTE, NULL, false, start, lexer->pos);
  }

  return initToken(TOK_GT, NULL, false, start, lexer->pos);
}

static inline Token _singleCharToken(Lexer *lexer, TokType tokenType) {
  Position start = lexer->pos;
  Position end = lexer->pos;
  
  advancePosition(&end, lexer->currChar);
  advanceLexer(lexer);
  
  return initToken(tokenType, NULL, false, start, end);
}

Token lexNextToken(Lexer *lexer, Error **error) {
  for (;;) {
    if (lexer->currChar == ' ' || lexer->currChar == '\t' || lexer->currChar == '\n') {
      advanceLexer(lexer);
      continue;
    }

    if (lexer->currChar == '#') {
      while (lexer->currChar != '\n' && lexer->currChar != '\0') advanceLexer(lexer);
      continue;
    }

    if (lexer->currChar == 0) {
      return (Token){.type = TOK_EOF};
    }

    switch (lexer->currChar) {
      case '"':  return makeStringLexer(lexer, error);
      case '\'': return makeCharLexer(lexer, error);
      case '!':  return makeNotEqualsToken(lexer, error);
      case '=':  return makeEqualsToken(lexer);
      case '<':  return makeLessThanToken(lexer);
      case '>':  return makeGreaterThanToken(lexer);

      case '+': return _singleCharToken(lexer, TOK_PLUS);
      case '-': return _singleCharToken(lexer, TOK_MINUS);
      case '[': return _singleCharToken(lexer, TOK_LBRACK);
      case ']': return _singleCharToken(lexer, TOK_RBRACK);
      case '{': return _singleCharToken(lexer, TOK_LCURLBRACK);
      case '}': return _singleCharToken(lexer, TOK_RCURLBRACK);
      case ':': return _singleCharToken(lexer, TOK_COLON);
      case ',': return _singleCharToken(lexer, TOK_COMMA);
      case '*': return _singleCharToken(lexer, TOK_MUL);
      case '/': return _singleCharToken(lexer, TOK_DIV);
      case '^': return _singleCharToken(lexer, TOK_POW);
      case '(': return _singleCharToken(lexer, TOK_LPAREN);
      case ')': return _singleCharToken(lexer, TOK_RPAREN);
      case '.': return _singleCharToken(lexer, TOK_DOT);

      default:
        if (_is_digit(lexer->currChar)) return makeNumberTokenLexer(lexer, error);
        if (_is_letter(lexer->currChar) || lexer->currChar == '_') return makeIdentifierLexer(lexer);

        {
          char details[4] = {'\'', lexer->currChar, '\'', '\0'};
          
          Position start = lexer->pos;
          advanceLexer(lexer);
          Position end = lexer->pos;
          
          if (error && *error == NULL)
            *error = initIllegalCharError(start, end, lexer->filename, details, lexer->text);
          
          return (Token){.type = TOK_INVALID};
        }
    }
  }
}

void freeLexer(Lexer *lexer) {
  // TODO: cleanup
}
