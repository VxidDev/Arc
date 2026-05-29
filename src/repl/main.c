#include "../../include/repl/input.h"
#include "../../include/utils.h"

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
#include "../../include/repl/readfile.h"

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
char *_INPUT_FILE = NULL;

String** argVect;
uint64_t argVectSize = 1;
uint64_t argVectCap = 64;

void appendArgv(String* s) {
  if (!s) return;

  if (argVectSize >= argVectCap) {
    argVectCap *= 2;

    argVect = realloc(argVect, sizeof(String*) * argVectCap);
  }

  argVect[argVectSize++] = s;
}

static void printObjInternal(Object* obj) {
  if (obj->type == OBJ_NUMBER_INT) {
    printf("%s%s%ld%s",
      COLOR(ANSI_BRIGHT_CYAN_FG),
      COLOR(ANSI_BOLD),
      ((Number*)obj)->as.i,
      COLOR(ANSI_RESET));

  } else if (obj->type == OBJ_NUMBER_FLOAT) {
    printf("%s%s%.*f%s",
      COLOR(ANSI_BRIGHT_CYAN_FG),
      COLOR(ANSI_BOLD),
      _FLOAT_PRECISION,
      ((Number*)obj)->as.f,
      COLOR(ANSI_RESET));

  } else if (obj->type == OBJ_STRING) {
    printf("%s%s%s%s",
      COLOR(ANSI_BRIGHT_GREEN_FG),
      COLOR(ANSI_BOLD),
      ((String*)obj)->value,
      COLOR(ANSI_RESET));

  } else if (obj->type == OBJ_LIST) {
    List* list = (List*)obj;

    putchar('[');

    for (uint64_t i = 0; i < list->size; i++) {
      printObjInternal(list->objects[i]);

      if (i + 1 < list->size)
        printf(", ");
    }

    putchar(']');
  }
}

void printObj(Object* obj) {
  printObjInternal(obj);
  putchar('\n');
}

void run(char *text, Error **error, unsigned long *size, SymbolTable* variables, char *filename) {
  Lexer *lexer = initLexer(stringDup(filename), text);

  if (!lexer) {
    printf("%sArc: %sFailed to initialize lexer.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
    return;
  }

  Token **tokens = makeTokensLexer(lexer, error, size);

  if (!tokens) {
    if (*error) {
      char *errStr = errorAsString(*error);
      printf("%s%s%s\n", COLOR(ANSI_BRIGHT_RED_FG), errStr, COLOR(ANSI_RESET));
      free(errStr);

      freeError(*error);
      *error = NULL;
    }

    freeLexer(lexer);
    return;
  }

  if (_DEBUG) {
    printf("\n%sTokens: %s", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_BLUE_FG));

    for (size_t i = 0; i < *size; i++) {
      printf("%s ", tokToString(tokens[i]->type));
    }

    if (_IS_COLORED) printf("%s\n", ANSI_RESET);
    else putchar('\n');
  }

  Parser* parser = initParser(tokens, *size, error);

  if (!parser) {
    freeTokens(tokens, *size);

    free(lexer->filename);
    freeLexer(lexer);

    return;
  }

  ASTNode* ast = parseProgram(parser);

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

    free(lexer->filename);
    freeLexer(lexer);

    return;
  }

  Object* result = visitNode(ast, filename, error, variables);

  if (!result) {
    freeAST(ast);
    freeTokens(tokens, *size);
    free(parser);

    free(lexer->filename);
    freeLexer(lexer);

    return;
  }

  printObj(result); 

  freeObject(result);

  freeAST(ast);
  freeTokens(tokens, *size);
  free(parser);

  free(lexer->filename);
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
  if (argc < 2) return; // repl

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      if (!_INPUT_FILE) {
        _INPUT_FILE = argv[i];
      } else {
        String* arg = initString(argv[i], strlen(argv[i]));
        appendArgv(arg);
      }
      continue;
    }

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
  char *userInput = NULL;
  SymbolTable *variables = createTable(1024, NULL);
  
  argVect = malloc(sizeof(String*) * argVectCap);

  if (!argVect) {
    freeTable(variables);
    printf("%sArc: %sFailed to initialize argv.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_BLUE_FG), COLOR(ANSI_RESET));

    return 1;
  }

  parseArguments(argc, argv);
  
  char *arg0raw = _INPUT_FILE ? _INPUT_FILE : argv[0];
  String* arg0 = initString(arg0raw, strlen(arg0raw));

  argVect[0] = arg0;

  Number* argumentCount = initInt(argVectSize);
  setTable(variables, "argc", (Object*)argumentCount);
  free(argumentCount);

  List* list = initList((Object**)argVect, argVectSize, argVectCap);
  
  free(argVect);

  setTable(variables, "argv", (Object*)list);
  freeObject((Object*)list);

  registerBuiltins(variables);

  char *code = NULL;

  if (_CODE) {
    code = _CODE;
  } else if (_INPUT_FILE) {
    code = readFile(_INPUT_FILE);

    if (!code) {
      return 1;
    }

    if (_DEBUG) {
      printf("%sArc: %s%sFile contents:%s\n%s%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BOLD), COLOR(ANSI_BRIGHT_BLUE_FG), COLOR(ANSI_WHITE_FG), code, COLOR(ANSI_RESET));
    }
  }

  if (code) {
    Error *error = NULL;
    unsigned long size = 0;

    run(code, &error, &size, variables, _INPUT_FILE ? _INPUT_FILE : "<stdin>");

    if (error) {
      char *errStr = errorAsString(error);
      printf("%s%s%s\n", COLOR(ANSI_BRIGHT_RED_FG), errStr, COLOR(ANSI_RESET));
      free(errStr);
      freeError(error);
    }

    if (!_CODE) {
      free(code);
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
    run(userInput, &error, &size, variables, "<stdin>");

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
