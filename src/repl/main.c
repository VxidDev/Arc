#include "../../include/repl/input.h"

#include "../../include/token.h"
#include "../../include/lexer.h"
#include "../../include/error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token** run(char *text, Error **error, unsigned long *size) {
  Lexer *lexer = initLexer("stdin", text);

  if (!lexer) {
    printf("\n[ ARC ] Failed to initialize lexer.\n");
    return NULL;
  }
  
  Token **tokens = makeTokensLexer(lexer, error, size);

  freeLexer(lexer);
  
  return tokens;
}

void freeTokens(Token **tokens, unsigned long size) {
  for (unsigned long i = 0; tokens[i]; i++) {
    if (tokens[i]->needsToBeFreed) free(tokens[i]->value);
    free(tokens[i]->type);
    free(tokens[i]);
  }

  free(tokens);
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
    Token **tokens = run(userInput, &error, &size);
    
    if (!tokens) {
      char *errStr = errorAsString(error); 

      printf("[ ARC - ERROR ] %s\n", errStr);
      free(userInput); 
      free(errStr);

      freeError(error);

      continue;
    }
    
    printf("[");

    for (unsigned long i = 0; i < size; i++) {
        if (i > 0) printf(", ");

        if (strcmp(tokens[i]->type, TOK_INT) == 0) {
            printf("%s:%d", tokens[i]->type, *(int*)tokens[i]->value);
        }
        else if (strcmp(tokens[i]->type, TOK_FLOAT) == 0) {
            printf("%s:%f", tokens[i]->type, *(double*)tokens[i]->value);
        }
        else {
            printf("%s", tokens[i]->type);
        }
    }

    printf("]\n"); 
    
    freeTokens(tokens, size);
    free(userInput); 
  } 

  return 0;
}
