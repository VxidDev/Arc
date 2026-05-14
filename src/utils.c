#include "../include/utils.h"
#include "../include/token.h"

#include <string.h>
#include <stdlib.h>

char *stringDup(const char *s) {
  size_t len = strlen(s) + 1;
  char *sDup = malloc(len);
  
  memcpy(sDup, s, len);

  return sDup;
}

char* tokToString(const TokType type) {
  switch (type) {
    case TOK_INT: return "INT";
    case TOK_FLOAT: return "FLOAT";

    case TOK_PLUS: return "PLUS";
    case TOK_MINUS: return "MINUS";
    case TOK_MUL: return "MUL";
    case TOK_DIV: return "DIV";
    case TOK_POW: return "POW";

    case TOK_LPAREN: return "LPAREN";
    case TOK_RPAREN: return "RPAREN";

    case TOK_EQ: return "EQ";
    case TOK_EE: return "EE";
    case TOK_NE: return "NE";
    case TOK_LT: return "LT";
    case TOK_GT: return "GT";
    case TOK_LTE: return "LTE";
    case TOK_GTE: return "GTE";

    case TOK_IDENTIFIER: return "IDENTIFIER";
    case TOK_KEYWORD: return "KEYWORD";
    case TOK_STRING: return "STRING";

    default: return "UNKNOWN";
  }
}
