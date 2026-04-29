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

bool _DEBUG;

void printAST(ASTNode* node) {
  if (!node) return;

  switch (node->type) {
    case NODE_NUMBER: {
      NumberNode* n = (NumberNode*)node;

      if (strcmp(n->token->type, TOK_INT) == 0) {
        printf("%s%d%s", ANSI_BRIGHT_YELLOW_FG, *(int*)n->token->value, ANSI_RESET);
      } else if (strcmp(n->token->type, TOK_FLOAT) == 0) {
        printf("%s%s%f%s", ANSI_DIM, ANSI_YELLOW_FG, *(double*)n->token->value, ANSI_RESET);
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

      putchar('(');
      printf("%s%s%s ", ANSI_BRIGHT_BLACK_FG, u->operTok->type, ANSI_RESET);
      printAST(u->node);
      putchar(')');

      break;
    }
  }
}

ASTNode* run(char *text, Error **error, unsigned long *size) {
  Lexer *lexer = initLexer("<stdin>", text);

  if (!lexer) {
    printf("%sArc: %sFailed to initialize lexer.%s\n", ANSI_CYAN_FG, ANSI_BRIGHT_RED_FG, ANSI_RESET);
    return NULL;
  }
  
  Token **tokens = makeTokensLexer(lexer, error, size);
  
  if (!tokens) return NULL;
  
  if (_DEBUG) {
    printf("\n%sTokens: %s", ANSI_CYAN_FG, ANSI_BRIGHT_BLUE_FG);

    for (size_t i = 0; i < *size; i++) {
      printf("%s ", tokens[i]->type);
    }

    printf("%s\n", ANSI_RESET);
  }
  
  Parser* parser = initParser(tokens, *size);

  if (!parser) {
    freeLexer(lexer);
    return NULL;
  }

  ASTNode* ast = parseParser(parser);
  
  if (_DEBUG) {
    printf("%sAST tree: ", ANSI_CYAN_FG);
    printAST(ast);
    printf("%s\n\n", ANSI_RESET);
  }

  Number* result = visitNode(ast);

  if (!result) {
    printf("%sArc: %sFailed to calculate result.%s\n", ANSI_CYAN_FG, ANSI_BRIGHT_RED_FG, ANSI_RESET);
    free(lexer);
    free(parser);
    free(tokens);

    return ast;
  }
  
  printf("%s%s%ld%s\n", ANSI_BRIGHT_CYAN_FG, ANSI_BOLD, result->value, ANSI_RESET);
  free(result);

  freeLexer(lexer);
  free(parser);
  free(tokens);

  return ast;
}

void parseArguments(int argc, char **argv) {
  if (argc < 1) return;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
      _DEBUG = true;
      break;
    }

    printf("%sArc: %sunknown argument \"%s\"%s\n", ANSI_CYAN_FG, ANSI_WHITE_FG, argv[i], ANSI_RESET);
    exit(1);
  }
}

int main(int argc, char **argv) {
  parseArguments(argc, argv);

  char *userInput = NULL;

  char prompt[] = "\033[36mArc > \033[0m";

  for (;;) {
    userInput = input(prompt);

    if (!userInput) {
      printf("%sArc: %sFailed to get user input.%s\n", ANSI_CYAN_FG, ANSI_BRIGHT_RED_FG, ANSI_RESET);
      break;
    }

    if (strcmp(userInput, "exit") == 0) {
      free(userInput);
      return 0;
    }
    
    Error *error = NULL;
    
    unsigned long size = 0; 
    ASTNode* ast = run(userInput, &error, &size);
    
    if (!ast) {
      char *errStr = errorAsString(error); 

      printf("%s%s%s\n", ANSI_BRIGHT_RED_FG, errStr, ANSI_RESET);
      free(userInput); 
      free(errStr);

      freeError(error);

      continue;
    }
    
    free(userInput); 
    freeAST(ast);
  } 

  return 0;
}
