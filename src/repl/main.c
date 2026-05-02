#include "../../include/repl/input.h"

#include "../../include/token.h"
#include "../../include/lexer.h"
#include "../../include/parser.h"
#include "../../include/node.h"
#include "../../include/error.h"
#include "../../include/interpretator.h"
#include "../../include/object.h"

#include "../../include/ansi-colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

bool _DEBUG = false;
int _FLOAT_PRECISION = 6;

void printAST(ASTNode* node) {
  if (!node) return;

  switch (node->type) {
    case NODE_NUMBER: {
      NumberNode* n = (NumberNode*)node;

      if (strcmp(n->token->type, TOK_INT) == 0) {
        printf("%s%d%s", ANSI_BRIGHT_YELLOW_FG, *(int*)n->token->value, ANSI_RESET);
      } else if (strcmp(n->token->type, TOK_FLOAT) == 0) {
        printf("%s%s%.*f%s", ANSI_DIM, ANSI_YELLOW_FG, _FLOAT_PRECISION, *(double*)n->token->value, ANSI_RESET);
      }

      break;
    }

    case NODE_BINOP: {
      BinOpNode* b = (BinOpNode*)node;

      putchar('(');
      printAST(b->leftNode);

      printf(" %s%s%s ", ANSI_BRIGHT_CYAN_FG, b->operTok->type, ANSI_RESET);

      printAST(b->rightNode);
      putchar(')');
      break;
    }

    case NODE_UNARYOP: {
      UnaryOpNode* u = (UnaryOpNode*)node;

      printf("(%s%s%s ", ANSI_BRIGHT_BLACK_FG, u->operTok->type, ANSI_RESET);
      printAST(u->node);
      putchar(')');

      break;
    }

    case NODE_VARASSIGN: {
      VarAssignNode* va = (VarAssignNode*)node;

      printf("%s[%s = ", ANSI_BRIGHT_MAGENTA_FG, va->identifier);

      printAST(va->value);

      printf("]%s", ANSI_RESET);
      break;
    } 

    case NODE_VARACCESS: {
      VarAccessNode* va = (VarAccessNode*)node;
      printf("%s[VAR-ACCESS:%s]%s", ANSI_BRIGHT_MAGENTA_FG, (char*)va->token->value, ANSI_RESET);
      break;
    }
  }
}

void run(char *text, Error **error, unsigned long *size, SymbolTable* variables) {
  Lexer *lexer = initLexer("<stdin>", text);

  if (!lexer) {
    printf("%sArc: %sFailed to initialize lexer.%s\n", ANSI_CYAN_FG, ANSI_BRIGHT_RED_FG, ANSI_RESET);
    return;
  }
  
  Token **tokens = makeTokensLexer(lexer, error, size);
  
  if (!tokens) {
    freeLexer(lexer);
    return;
  }
  
  if (_DEBUG) {
    printf("\n%sTokens: %s", ANSI_CYAN_FG, ANSI_BRIGHT_BLUE_FG);

    for (size_t i = 0; i < *size; i++) {
      printf("%s ", tokens[i]->type);
    }

    printf("%s\n", ANSI_RESET);
  }
  
  Parser* parser = initParser(tokens, *size, error);

  if (!parser) {
    freeLexer(lexer);
    return;
  }

  ASTNode* ast = parseParser(parser);
  
  if (_DEBUG) {
    printf("%sAST tree: ", ANSI_CYAN_FG);
    printAST(ast);
    printf("%s\n\n", ANSI_RESET);
  }

  if (!ast) {
    printf("%sArc: %sFailed to construct AST.%s\n", ANSI_CYAN_FG, ANSI_BRIGHT_RED_FG, ANSI_RESET);

    if (*error) {
      char *errStr = errorAsString(*error);
      printf("%s%s%s\n", ANSI_BRIGHT_RED_FG, errStr, ANSI_RESET);
      free(errStr);

      freeError(*error);
      *error = NULL;
    }

    freeTokens(tokens, *size);
    free(parser);
    freeLexer(lexer);
    
    return;
  }
  
  Number* result = visitNode(ast, "<stdin>", error, variables);

  if (!result) {
    freeAST(ast);
    freeTokens(tokens, *size);
    free(parser);
    freeLexer(lexer);

    return;
  }
  
  if (result->base.type == OBJ_NUMBER_INT) {
    printf("%s%s%ld%s\n", ANSI_BRIGHT_CYAN_FG, ANSI_BOLD, result->as.i, ANSI_RESET);
  } else if (result->base.type == OBJ_NUMBER_FLOAT){
    printf("%s%s%.*f%s\n", ANSI_BRIGHT_CYAN_FG, ANSI_BOLD, _FLOAT_PRECISION, result->as.f, ANSI_RESET);
  }

  free(result);
  
  freeAST(ast);
  freeTokens(tokens, *size);
  free(parser);
  freeLexer(lexer);
}

int parseInt(const char *s, int *out) {
  char *end = NULL;
  errno = 0;

  long val = strtol(s, &end, 10);

  if (errno != 0) return 0; // overflow / underflow
  
  while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') end++;

  if (*end != '\0') return 0; // invalid chars 
  if (val < INT_MIN || val > INT_MAX) return 0;

  *out = (int)val;
  return 1;
}

void parseArguments(int argc, char **argv) {
  if (argc < 2) return;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
      _DEBUG = true;
      continue;
    } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--float-precision") == 0) {
      if (i + 1 >= argc) {
        printf("%sArc: %sprecision cannot be empty.%s\n", ANSI_CYAN_FG, ANSI_BRIGHT_RED_FG, ANSI_RESET);
        exit(1);
      }

      int precision;

      if (!parseInt(argv[++i], &precision)) {
        printf("%sArc: %sprecision must be a valid integer%s\n", ANSI_CYAN_FG, ANSI_BRIGHT_RED_FG, ANSI_RESET);
        exit(1);
      }

      _FLOAT_PRECISION = precision;
    } else {
      printf("%sArc: %sunknown argument \"%s\"%s\n", ANSI_CYAN_FG, ANSI_WHITE_FG, argv[i], ANSI_RESET);
      exit(1);
    } 
  }
}

int main(int argc, char **argv) {
  parseArguments(argc, argv);

  char *userInput = NULL;
  SymbolTable *variables = createTable(1024, NULL);

  char prompt[] = "\033[36mArc > \033[0m";

  for (;;) {
    userInput = input(prompt);

    if (!userInput) {
      printf("%sArc: %sFailed to get user input.%s\n", ANSI_CYAN_FG, ANSI_BRIGHT_RED_FG, ANSI_RESET);
      freeTable(variables);
      break;
    }
    
    if (userInput[0] == '\0') {
      free(userInput);
      continue;
    }

    if (strcmp(userInput, "exit") == 0) {
      free(userInput);
      freeTable(variables);
      return 0;
    } else if (strcmp(userInput, "clear") == 0) {
      printf("\033[2J\033[H");
      free(userInput);
      continue;
    }
    
    Error *error = NULL;
    
    unsigned long size = 0; 
    run(userInput, &error, &size, variables);
     
    if (error) {
      char *errStr = errorAsString(error);
      printf("%s%s%s\n", ANSI_BRIGHT_RED_FG, errStr, ANSI_RESET);
      free(errStr);
      freeError(error);
    }
    
    free(userInput);
  } 

  return 0;
}
