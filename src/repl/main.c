#include "../../include/repl/input.h"

#include "../../include/token.h"
#include "../../include/lexer.h"
#include "../../include/parser.h"
#include "../../include/node.h"
#include "../../include/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printAST(ASTNode* node) {
  if (!node) return;

  switch (node->type) {
    case NODE_NUMBER: {
      NumberNode* n = (NumberNode*)node;

      if (strcmp(n->token->type, TOK_INT) == 0) {
        printf("%d", *(int*)n->token->value);
      } else if (strcmp(n->token->type, TOK_FLOAT) == 0) {
        printf("%f", *(double*)n->token->value);
      }

      break;
    }

    case NODE_BINOP: {
      BinOpNode* b = (BinOpNode*)node;

      printf("(");
      printAST(b->leftNode);

      printf(" %s ", b->operTok->type);

      printAST(b->rightNode);
      printf(")");
      break;
    }

    case NODE_UNARYOP: {
      UnaryOpNode* u = (UnaryOpNode*)node;

      printf("(");
      printf("%s ", u->operTok->type);
      printAST(u->node);
      printf(")");

      break;
    }
  }
}

ASTNode* run(char *text, Error **error, unsigned long *size) {
  Lexer *lexer = initLexer("<stdin>", text);

  if (!lexer) {
    printf("\n[ ARC ] Failed to initialize lexer.\n");
    return NULL;
  }
  
  Token **tokens = makeTokensLexer(lexer, error, size);
  
  if (!tokens) return NULL;

  for (size_t i = 0; i < *size; i++) {
    printf("%s ", tokens[i]->type);
  }

  putchar('\n');

  Parser* parser = initParser(tokens, *size);

  if (!parser) {
    freeLexer(lexer);
    return NULL;
  }

  ASTNode* ast = parseParser(parser);

  printAST(ast);
  putchar('\n');
  
  freeLexer(lexer);
  free(parser);
  free(tokens);

  return ast;
}

int main(int argc, char **argv) {
  char *userInput = NULL;

  for (;;) {
    userInput = input("Arc > ");

    if (!userInput) {
      printf("\n[ ARC - FATAL ] Failed to get user input.\n");
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

      printf("[ ARC - ERROR ] %s\n", errStr);
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
