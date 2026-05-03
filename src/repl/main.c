#include "../../include/repl/input.h"

#include "../../include/token.h"
#include "../../include/lexer.h"
#include "../../include/parser.h"
#include "../../include/node.h"
#include "../../include/error.h"
#include "../../include/interpretator.h"
#include "../../include/object.h"

#include "../../include/ansi-colors.h"
#include "../../include/repl/repl.h"
#include "../../include/repl/printast.h"
#include "../../include/repl/help.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

bool _DEBUG = false;
bool _IS_COLORED = true;
int _FLOAT_PRECISION = 6;
char *_CODE = NULL;

void run(char *text, Error **error, unsigned long *size, SymbolTable* variables) {
  Lexer *lexer = initLexer("<stdin>", text);

  if (!lexer) {
    printf("%sArc: %sFailed to initialize lexer.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
    return;
  }
  
  Token **tokens = makeTokensLexer(lexer, error, size);
  
  if (!tokens) {
    freeLexer(lexer);
    return;
  }
  
  if (_DEBUG) {
    printf("\n%sTokens: %s", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_BLUE_FG));

    for (size_t i = 0; i < *size; i++) {
      printf("%s ", tokens[i]->type);
    }

    if (_IS_COLORED) printf("%s\n", ANSI_RESET);
    else putchar('\n');
  }
  
  Parser* parser = initParser(tokens, *size, error);

  if (!parser) {
    freeLexer(lexer);
    return;
  }

  ASTNode* ast = parseParser(parser);
  
  if (_DEBUG) {
    printf("%sAST tree: ", COLOR(ANSI_CYAN_FG));
    printAST(ast);
    if (_IS_COLORED) printf("%s\n\n", ANSI_RESET);
    else puts("\n"); // puts adds an additional newline
  }

  if (!ast) {
    if (_DEBUG) printf("%sArc: %sFailed to construct AST.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));

    if (*error) {
      char *errStr = errorAsString(*error);
      printf("%s%s%s\n", COLOR(ANSI_BRIGHT_RED_FG), errStr, COLOR(ANSI_RESET));
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
    printf("%s%s%ld%s\n", COLOR(ANSI_BRIGHT_CYAN_FG), COLOR(ANSI_BOLD), result->as.i, COLOR(ANSI_RESET));
  } else if (result->base.type == OBJ_NUMBER_FLOAT){
    printf("%s%s%.*f%s\n", COLOR(ANSI_BRIGHT_CYAN_FG), COLOR(ANSI_BOLD), _FLOAT_PRECISION, result->as.f, COLOR(ANSI_RESET));
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
        printf("%sArc: %sprecision cannot be empty.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        exit(1);
      }

      int precision;

      if (!parseInt(argv[++i], &precision)) {
        printf("%sArc: %sprecision must be a valid integer%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        exit(1);
      }

      _FLOAT_PRECISION = precision;
    } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--code") == 0) {
      if (i + 1 >= argc) {
        printf("%sArc: %scode cannot be empty.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        exit(1);
      }

      _CODE = argv[++i];
    } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--disable-colored-formatting") == 0){
      _IS_COLORED = false;
      continue;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printHelp();
      exit(0);
    } else {
      printf("%sArc: %sunknown argument \"%s\"%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_WHITE_FG), argv[i], COLOR(ANSI_RESET));
      exit(1);
    } 
  }
}

int main(int argc, char **argv) {
  parseArguments(argc, argv);

  char *userInput = NULL;
  SymbolTable *variables = createTable(1024, NULL);

  if (_CODE) {
    Error *error = NULL;
    
    unsigned long size = 0; 
    run(_CODE, &error, &size, variables);
     
    if (error) {
      char *errStr = errorAsString(error);
      printf("%s%s%s\n", COLOR(ANSI_BRIGHT_RED_FG), errStr, COLOR(ANSI_RESET));
      free(errStr);
      freeError(error);
    }

    freeTable(variables);

    return 0;
  }
  
  char *prompt;

  if (_IS_COLORED) prompt = "\033[36mArc > \033[0m";
  else prompt = "Arc > ";

  for (;;) {
    userInput = input(prompt);

    if (!userInput) {
      printf("%sArc: %sFailed to get user input.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
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
      printf("%s%s%s\n", COLOR(ANSI_BRIGHT_RED_FG), errStr, COLOR(ANSI_RESET));
      free(errStr);
      freeError(error);
    }
    
    free(userInput);
  } 

  return 0;
}
