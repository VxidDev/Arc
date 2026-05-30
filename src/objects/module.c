#include "../../include/object.h"

#include <stdlib.h>

Module* initModule(ASTNode* astTree, Lexer* lexer, Parser* parser, char *fileContent, Token* tokens, size_t tokenAmount) {
  if (!astTree || !lexer || !parser) return NULL;

  Module* module = malloc(sizeof(Module));

  if (!module) return NULL;

  module->base.type = OBJ_MODULE;

  module->astTree = astTree;
  module->lexer = lexer;
  module->parser = parser;
  module->fileContent = fileContent;

  module->tokens = tokens;
  module->tokenAmount = tokenAmount;

  return module;
}
