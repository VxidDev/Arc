#include "../../include/repl/input.h"
#include "../../include/utils.h"

#include "../../include/token.h"
#include "../../include/lexer.h"
#include "../../include/parser.h"
#include "../../include/node.h"
#include "../../include/error.h"

#include "../../include/object.h"
#include "../../include/vm.h"

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
bool _PRINT_LAST_RESULT = false;
int POOL_SIZE = 1024;
bool _SKIP_EVAL = false;
size_t ARENA_BLOCK_SIZE = (256 * 1024); // 256 Kb
SymbolTable* variables = NULL;
int exitcode = 0;
bool _CLEANUP = false;

String** argVect;
uint64_t argVectSize = 1;
uint64_t argVectCap = 64;

static inline void _exit(int code) {
  if (_CLEANUP) {
    if (variables) freeTable(variables);

    if (argVect) {
      for (uint64_t i = 1; i < argVectSize; i++) {
        freeObject((Object*)argVect[i]);
      }

      free(argVect);
    }

    freeMemPools();
    freeArenas();
  }

  exit(code);
}

static inline void appendArgv(String* s) {
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
  } else if (obj->type == OBJ_RETURN) {
    Return* ret = (Return*)obj;

    printObjInternal(ret->value);
  } else if (obj->type == OBJ_FILE) {
    File* file = (File*)obj;
    printf("FILE:%s|%s", file->fname, file->fmod);
  } 
}

void printObj(Object* obj) {
  printObjInternal(obj);
  putchar('\n');
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

static inline void run(char *text, Error **error, unsigned long *size, SymbolTable* variables, char *filename) {
  Lexer *lexer = initLexer(stringDup(filename), text);

  if (!lexer) {
    printf("%sArc: %sFailed to initialize lexer.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
    return;
  }

  Token *tokens = makeTokensLexer(lexer, error, size);

  if (!tokens) {
    if (*error) {
      char *errStr = errorAsString(*error);
      printf("%s%s%s\n", COLOR(ANSI_BRIGHT_RED_FG), errStr, COLOR(ANSI_RESET));
      free(errStr);

      freeError(*error);
      *error = NULL;
    }
    
    return;
  }

  if (_DEBUG) {
    printf("\n%sTokens: %s", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_BLUE_FG));

    for (size_t i = 0; i < *size; i++) {
      printf("%s ", tokToString(tokens[i].type));
    }

    if (_IS_COLORED) printf("%s\n", ANSI_RESET);
    else putchar('\n');
  }

  Parser* parser = initParser(tokens, *size, error, text, filename);

  if (!parser) {
    freeTokens(tokens, *size);
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
    return;
  }

  Chunk* chunk = compileAST(ast, error, filename, text);

  if (!chunk) {
    printf("%sArc: %sFailed to compile AST tree to bytecode.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
    return;
  }

  if (_DEBUG) {
    disassembleChunk(chunk, "main");
  }
  
  if (!_SKIP_EVAL) {
    //Interpretator* interpretator = initInterpretator(filename, text, error, variables);
    //Object* result = visitNode(ast, interpretator);

    VM* vm = initVM(chunk, variables, error, filename, text);
    if (_DEBUG) printf("[vm] Starting vmRun...\n");
    Object* result = vmRun(vm);
    if (_DEBUG) printf("[vm] vmRun returned. Result: %p, Error: %p\n", (void*)result, (void*)*error);

    deinitVM(vm);

    if (!result) {
      if (*error && (*error)->details[0] != '@') { 
        char *errStr = errorAsString(*error);
        printf("%s%s%s\n", COLOR(ANSI_BRIGHT_RED_FG), errStr, COLOR(ANSI_RESET));
        free(errStr);

        freeError(*error);
        *error = NULL;
      } else if (*error && (*error)->details[0] == '@') {
        char *exitCodeStr = (*error)->details + 1;
        
        int exitcodeInt;

        if (!parseInt(exitCodeStr, &exitcodeInt)) {
          printf("%sArc: %sExit code is expected to be a valid integer, defaulting to 1.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
          exitcodeInt = 1;
        }

        exitcode = exitcodeInt; 
      } else {
        printf("%sArc: %sFailed to execute bytecode.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        return;
      }

      freeTokens(tokens, *size);
      return;
    }

    if (_PRINT_LAST_RESULT) printObj(result); 

    freeObject(result);
  }

  freeChunk(chunk);
  freeTokens(tokens, *size);
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
    } else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "--skip-evaluation") == 0) {
      _SKIP_EVAL = true;
      continue;
    } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--float-precision") == 0) {
      if (i + 1 >= argc) {
        printf("%sArc: %sprecision cannot be empty.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        _exit(1);
      }

      int precision;

      if (!parseInt(argv[++i], &precision)) {
        printf("%sArc: %sprecision must be a valid integer%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        _exit(1);
      }

      _FLOAT_PRECISION = precision;
    } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mempool-size") == 0) {
      if (i + 1 >= argc) {
        printf("%sArc: %smemory pool size cannot be empty.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        _exit(1);
      }

      int mempoolSize;

      if (!parseInt(argv[++i], &mempoolSize)) {
        printf("%sArc: %smemory pool size must be a valid integer%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        _exit(1);
      }

      if (mempoolSize < 8) {
        printf("%sArc: %smemory pool size must be greater than or equal to 8%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        _exit(1);
      }

      POOL_SIZE = mempoolSize;

    } else if (strcmp(argv[i], "-A") == 0 || strcmp(argv[i], "--arena-block-size") == 0) {
      if (i + 1 >= argc) {
        printf("%sArc: %sarena block size cannot be empty.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        _exit(1);
      }

      int arenaBlockSize;

      if (!parseInt(argv[++i], &arenaBlockSize)) {
        printf("%sArc: %sarena block size must be a valid integer%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        _exit(1); 
      }

      if (arenaBlockSize < 4) {
        printf("%sArc: %sarena block size must be greater than or equal to 4 Kb.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        _exit(1);
      }

      ARENA_BLOCK_SIZE = arenaBlockSize * 1024; // convert to Kbs
    } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--code") == 0) {
      if (i + 1 >= argc) {
        printf("%sArc: %scode cannot be empty.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));
        _exit(1);
      }

      _CODE = argv[++i];
    } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--disable-colored-formatting") == 0){
      _IS_COLORED = false;
      continue;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printHelp();
      _exit(0);
    } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--last-result") == 0) {
      _PRINT_LAST_RESULT = true;
      continue;
    } else if (strcmp(argv[i], "-C") == 0 || strcmp(argv[i], "--cleanup") == 0) {
      _CLEANUP = true;
      continue;
    } else {
      printf("%sArc: %sunknown argument \"%s\"%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_WHITE_FG), argv[i], COLOR(ANSI_RESET));
      _exit(1);
    }
  }
}

int main(int argc, char **argv) {
  char *userInput = NULL; 
  
  argVect = malloc(sizeof(String*) * argVectCap);

  if (!argVect) {
    printf("%sArc: %sFailed to initialize argv.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_BLUE_FG), COLOR(ANSI_RESET));

    return 1;
  } 

  parseArguments(argc, argv);
  
  initMemPools();
  initArenas();

  variables = createTable(1024, NULL);

  if (_DEBUG) { 
    printf("[debug] memory pool size: %d bytes\n", POOL_SIZE);
    printf("[debug] arena block size: %zu bytes\n", ARENA_BLOCK_SIZE);
  }
  
  char *arg0raw = _INPUT_FILE ? _INPUT_FILE : argv[0];
  String* arg0 = initString(arg0raw, strlen(arg0raw));

  argVect[0] = arg0;

  Number* argumentCount = initInt(argVectSize);
  setTable(variables, internIdentifier("argc", 4), VAL_OBJ((Object*)argumentCount));

  List* list = initList((Object**)argVect, argVectSize, argVectCap);
  
  free(argVect);

  setTable(variables, internIdentifier("argv", 4), VAL_OBJ((Object*)list));

  registerBuiltins(variables);

  char *code = NULL;

  if (_CODE) {
    code = _CODE;
  } else if (_INPUT_FILE) {
    code = readFile(_INPUT_FILE);

    if (!code) {
      if (_CLEANUP) {
        freeTable(variables);
        freeMemPools();
        freeArenas();
      }
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
    
    if (_CLEANUP) {
      freeTable(variables);
      freeMemPools();
      freeArenas();
    }

    return exitcode;
  }

  char *prompt;

  if (_IS_COLORED) prompt = "\033[36mArc > \033[0m";
  else prompt = "Arc > ";

  for (;;) {
    userInput = input(prompt);

    if (!userInput) {
      printf("%sArc: %sFailed to get user input.%s\n", COLOR(ANSI_CYAN_FG), COLOR(ANSI_BRIGHT_RED_FG), COLOR(ANSI_RESET));

      if (_CLEANUP) {
        freeTable(variables);
        freeArenas();
        freeMemPools();
      }

      break;
    }

    if (userInput[0] == '\0') {
      free(userInput);
      continue;
    }

    if (strcmp(userInput, "exit") == 0) {
      if (_CLEANUP) {
        free(userInput);
        freeTable(variables);
        freeMemPools();
        freeArenas();
      }
      return 0;
    } else if (strcmp(userInput, "clear") == 0) {
      printf("\033[2J\033[H");
      free(userInput);
      continue;
    }

    Error *error = NULL;

    unsigned long size = 0;
    run(userInput, &error, &size, variables, "<stdin>");

    if (error && error->details[0] != '@') {
      char *errStr = errorAsString(error);
      printf("%s%s%s\n", COLOR(ANSI_BRIGHT_RED_FG), errStr, COLOR(ANSI_RESET));
      free(errStr);
      freeError(error);

      error = NULL;
    } else if (error && error->details[0] == '@') {
      if (_CLEANUP) {
        freeTable(variables);
        freeMemPools();
        freeArenas();
        free(userInput);
        freeError(error);
      }

      return exitcode;
    }

    free(userInput);
  }

  return 0;
}
