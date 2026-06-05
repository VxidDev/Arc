#include "../include/parser.h"
#include "../include/token.h"
#include "../include/utils.h"

#include "../include/memarena.h"

#include <stdlib.h>
#include <string.h>

ParserCheckpoint saveParser(Parser* parser) {
  return (ParserCheckpoint){
    .tokenIndex = parser->tokenIndex,
    .currentToken = parser->currentToken
  };
}

void rewindParser(Parser* parser, ParserCheckpoint checkpoint) {
  parser->tokenIndex = checkpoint.tokenIndex;
  parser->currentToken = checkpoint.currentToken;
}

Parser* initParser(Token* tokens, const unsigned long tokenAmount, Error **error) {
  if (!tokens || !error) return NULL;

  Parser* parser = arenaAlloc(parseArena, sizeof(Parser));

  if (!parser) return NULL;

  parser->tokens = tokens;
  parser->tokenAmount = tokenAmount;
  parser->tokenIndex = -1;
  parser->currentToken = (Token){.type = TOK_EOF};

  parser->error = error;

  advanceParser(parser);

  return parser;
}

Token advanceParser(Parser* parser) {
  if (!parser) return (Token){.type = TOK_EOF};

  parser->tokenIndex++;

  if ((size_t)parser->tokenIndex < parser->tokenAmount) {
    parser->currentToken = parser->tokens[parser->tokenIndex];
  } else {
    parser->currentToken = (Token){.type = TOK_EOF};
  }

  return parser->currentToken;
}

ASTNode* exprParser(Parser* parser);
ASTNode* termParser(Parser* parser);
ASTNode* factorParser(Parser* parser);
ASTNode* andOrParser(Parser* parser);
ASTNode* compExprParser(Parser* parser);
ASTNode* postfixParser(Parser* parser);
ASTNode* blockParser(Parser* parser);

ASTNode* atomParser(Parser* parser) {
  if (!parser || parser->currentToken.type == TOK_EOF) return NULL;

  Token token = parser->currentToken;

  if ((token.type == TOK_PLUS) || (token.type == TOK_MINUS) || (token.type == TOK_NOT)) {
    Token tok = token; // safe copy

    advanceParser(parser);

    ASTNode* expr = atomParser(parser);

    if (!expr) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(tok.start, tok.end, tok.start.filename, "Expression expected");
      return NULL;
    }

    return (ASTNode*)initUnaryOpNode(token, expr);
  }

  if (token.type == TOK_STRING) {
    advanceParser(parser);
    return (ASTNode*)initStringNode(token);
  }

  if ((token.type == TOK_INT) || (token.type == TOK_FLOAT)) {
    advanceParser(parser);
    return (ASTNode*)initNumberNode(token);
  } else if (token.type == TOK_IDENTIFIER) {
    advanceParser(parser);
    return (ASTNode*)initVarAccessNode(token);
  } else if (token.type == TOK_LPAREN) {
    advanceParser(parser);

    Token tok = token; // safe copy

    ASTNode* expr = andOrParser(parser);

    if (!expr) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(tok.start, tok.end, tok.start.filename, "Expression expected");
      return NULL;
    }

    if (parser->currentToken.type != TOK_EOF && (parser->currentToken.type == TOK_RPAREN)) {
      advanceParser(parser);
      return expr;
    } else {
      if (*parser->error == NULL) *parser->error = initSyntaxError(token.start, token.end, token.start.filename, "Expression expected");
      return NULL;
    }
  }

  if (token.type == TOK_LBRACK) {
    Token start = token;

    advanceParser(parser);
    
    if (parser->currentToken.type == TOK_EOF) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(token.start, token.end, token.start.filename, "Unexpected EOF.");
      return NULL;
    }

    uint64_t size = 0;
    uint64_t capacity = 64;

    ASTNode** objects = arenaAlloc(parseArena, capacity * sizeof(ASTNode*));

    if (!objects) {
      return NULL;
    }

    while (parser->currentToken.type != TOK_EOF && parser->currentToken.type != TOK_RBRACK) {
      token = parser->currentToken;
      ASTNode* val = andOrParser(parser);

      if (!val) {
        return NULL;
      }

      if (size >= capacity) {
        size_t oldcap = capacity;
        capacity *= 2;

        void *tmp = arenaRealloc(parseArena, objects, oldcap * sizeof(ASTNode*), capacity * sizeof(ASTNode*));

        if (!tmp) {
          return NULL;
        }

        objects = tmp;
      }

      objects[size++] = val;

      if (parser->currentToken.type == TOK_COMMA) {
        advanceParser(parser);

        continue;
      }

      if (parser->currentToken.type != TOK_EOF && parser->currentToken.type != TOK_RBRACK) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(parser->currentToken.start, parser->currentToken.end, parser->currentToken.start.filename, "Expected ',' or ']'.");

        return NULL;
      }
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_RBRACK) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(start.start, token.end, start.start.filename, "Unterminated list: expected ']'.");

      return NULL;
    }

    Token end = parser->currentToken;

    advanceParser(parser);

    objects[size] = NULL;

    return (ASTNode*)initListNode(start, end, objects, size, capacity);
  }

  if (*parser->error == NULL) *parser->error = initSyntaxError(token.start, token.end, token.start.filename, "Expression expected");
  return NULL;
}

ASTNode* powerParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = postfixParser(parser);
  if (!left) return NULL; // error is already set

  if (parser->currentToken.type != TOK_EOF && (parser->currentToken.type == TOK_POW)) {
    Token opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = powerParser(parser);

    if (!right) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(opTok.start, opTok.end, opTok.start.filename, "Expression expected after '^'");
      return NULL;
    }

    return (ASTNode*)initBinOpNode(left, opTok, right);
  }

  return left;
}

ASTNode* blockParser(Parser* parser) {
  size_t size = 0;
  size_t capacity = 64;

  ASTNode** statements = arenaAlloc(parseArena, sizeof(ASTNode*) * capacity);

  if (!statements) return NULL;

  while (
    parser->currentToken.type != TOK_EOF &&
    parser->currentToken.type != TOK_ELIF &&
    parser->currentToken.type != TOK_ELSE &&
    parser->currentToken.type != TOK_END && 
    parser->currentToken.type != TOK_CATCH
  ) {
    ASTNode* stmt = andOrParser(parser);

    if (!stmt) {
      return NULL;
    }

    if (size >= capacity) {
      size_t oldcap = capacity;
      capacity *= 2;

      void* tmp = arenaRealloc(parseArena, statements, oldcap * sizeof(ASTNode*), sizeof(ASTNode*) * capacity);

      if (!tmp) {
        return NULL;
      }

      statements = tmp;
    }

    statements[size++] = stmt;
  }

  statements[size] = NULL;

  return (ASTNode*)initProgramNode(statements, size);
}

ASTNode* postfixParser(Parser* parser) {
  ASTNode* node = atomParser(parser);
  if (!node) return NULL;

  while (parser->currentToken.type != TOK_EOF && (parser->currentToken.type == TOK_LPAREN)) {
    Position start = parser->currentToken.start;
    Position end = parser->currentToken.end;

    advanceParser(parser); // skip '('
    
    end = parser->currentToken.end; 

    size_t size = 0;
    size_t capacity = 16;

    ASTNode** args = arenaAlloc(parseArena, sizeof(ASTNode*) * capacity);

    if (!args) return NULL;

    if (parser->currentToken.type != TOK_EOF && parser->currentToken.type == TOK_RPAREN) {
      advanceParser(parser);
      return (ASTNode*)initFunctionCallNode(node, args, 0, start, end);
    }

    while (parser->currentToken.type != TOK_EOF && parser->currentToken.type != TOK_RPAREN) {
      end = parser->currentToken.end;
      ASTNode* arg = andOrParser(parser);

      if (!arg) {
        return NULL;
      }

      if (size >= capacity) {
        size_t oldcap = capacity;
        capacity *= 2;
        args = arenaRealloc(parseArena, args, oldcap * sizeof(ASTNode*), sizeof(ASTNode*) * capacity);
      }

      args[size++] = arg;

      if (parser->currentToken.type != TOK_EOF && parser->currentToken.type == TOK_COMMA) {
        end = parser->currentToken.end;
        advanceParser(parser);
      }
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_RPAREN) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(start, end, start.filename, "Expected ')'.");

      return NULL;
    }

    advanceParser(parser);

    return (ASTNode*)initFunctionCallNode(node, args, size, start, end);
  }

  while (parser->currentToken.type != TOK_EOF && (parser->currentToken.type == TOK_LBRACK)) {
    Position start = parser->currentToken.start;
    Position end = parser->currentToken.end;

    advanceParser(parser); // skip '['
    
    if (parser->currentToken.type == TOK_EOF) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(start, end, start.filename, "Expression expected."); 

      return NULL;
    }

    end = parser->currentToken.end;

    ASTNode* index = andOrParser(parser);

    if (!index) {
      return NULL;
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_RBRACK) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(start, end, start.filename, "Expected ']'");
      return NULL;
    }
    
    end = parser->currentToken.end;

    advanceParser(parser); // skip ']'
  
    node = (ASTNode*)initIndexNode(node, index, start, end);
  }

  return node;
}

ASTNode* factorParser(Parser* parser) {
  if (!parser || parser->currentToken.type == TOK_EOF) return NULL;

  Token token = parser->currentToken;

  if (token.type == TOK_EOF) return NULL;

  if ((token.type == TOK_PLUS) || (token.type == TOK_MINUS)) {
    Token opTok = token;

    advanceParser(parser);
    ASTNode* factor = factorParser(parser);

    if (!factor) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(opTok.start, opTok.end, opTok.start.filename, "Expression expected");
      return NULL;
    }

    ASTNode* unaryOpNode = (ASTNode*)initUnaryOpNode(token, factor);
    return unaryOpNode;
  }

  return powerParser(parser);
}

ASTNode* termParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = factorParser(parser);
  if (!left) return NULL; // error is already set

  while (parser->currentToken.type != TOK_EOF && ((parser->currentToken.type == TOK_MUL) || (parser->currentToken.type == TOK_DIV))) {

    Token opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = factorParser(parser);

    if (!right) { // error is already set
      if (*parser->error == NULL) *parser->error = initSyntaxError(opTok.start, opTok.end, opTok.start.filename, "Expression expected");
      return NULL;
    }

    left = (ASTNode*)initBinOpNode(left, opTok, right);

    if (!left) {
      return NULL;
    }
  }

  return left;
}

ASTNode* exprParser(Parser* parser) {
  if (!parser) return NULL;

  if (parser->currentToken.type == TOK_RPAREN) {
    if (*parser->error == NULL) {
      *parser->error = initSyntaxError(parser->currentToken.start, parser->currentToken.end, parser->currentToken.start.filename, "Unexpected ')'");
    }

    return NULL;
  }

  if (parser->currentToken.type == TOK_EOF) {
    // if (*parser->error == NULL); Will be implemented after addition of EOF token <- I'm too lazy to add that for now :[
    return NULL;
  }

  if (parser->currentToken.type == TOK_FOR) {
    Token forTok = parser->currentToken;

    advanceParser(parser);

    if (parser->currentToken.type != TOK_IDENTIFIER) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(forTok.start, forTok.end, forTok.start.filename, "Expected identifier after keyword 'FOR'.");
      return NULL;
    }

    Token identTok = parser->currentToken;

    advanceParser(parser);

    if (parser->currentToken.type != TOK_IN) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(identTok.start, identTok.end, identTok.start.filename, "Expected 'IN' after identifier.");
      return NULL;
    }

    Token inTok = parser->currentToken;

    advanceParser(parser);

    if (parser->currentToken.type == TOK_EOF) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(inTok.start, inTok.end, inTok.start.filename, "Expected expression after 'IN'.");
    }

    ASTNode* iterable = andOrParser(parser);
    if (!iterable) return NULL;

    if (parser->currentToken.type != TOK_THEN) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(inTok.start, inTok.end, inTok.start.filename, "Expected 'THEN' after iterable.");
        return NULL;
    }

    Token thenTok = parser->currentToken;

    advanceParser(parser); // Skip THEN 
    
    if (parser->currentToken.type == TOK_EOF) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(thenTok.start, thenTok.end, thenTok.start.filename, "Expected expression after 'THEN'.");
      return NULL;
    }

    ASTNode* body = blockParser(parser);

    if (!body) { 
      return NULL;
    }

    if (parser->currentToken.type != TOK_END) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(thenTok.start, thenTok.end, thenTok.start.filename, "Expected 'END' after body.");
        return NULL;
    }

    advanceParser(parser); // skip END

    return (ASTNode*)initForNode(forTok, identTok, iterable, body);
  }

  if (parser->currentToken.type == TOK_BREAK) {
    ASTNode* node = (ASTNode*)initBreakNode(parser->currentToken);

    advanceParser(parser); // Skip BREAK
    
    return node;
  }

  if (parser->currentToken.type == TOK_CONTINUE) {
    ASTNode* node = (ASTNode*)initContinueNode(parser->currentToken);

    advanceParser(parser); // Skip CONTINUE 
    
    return node;
  }

  if (parser->currentToken.type == TOK_TRY) {
    Token tryTok = parser->currentToken;

    advanceParser(parser); // skip TRY 
    
    if (parser->currentToken.type == TOK_EOF) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(tryTok.start, tryTok.end, tryTok.start.filename, "Expected expression.");
      return NULL;
    }

    ASTNode* body = blockParser(parser);

    if (!body) { // Error is already set 
      return NULL;
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_CATCH) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(tryTok.start, tryTok.end, tryTok.start.filename, "Expected 'CATCH'.");
      return NULL;
    }
    
    Token catchTok = parser->currentToken;

    advanceParser(parser); // Skip CATCH
    
    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_IDENTIFIER) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(catchTok.start, catchTok.end, catchTok.start.filename, "Expected identifier after 'CATCH'.");
      return NULL;
    }

    Token errIdentifier = parser->currentToken;

    advanceParser(parser); // skip IDENTIFIER 

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_THEN) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(catchTok.start, catchTok.end, catchTok.start.filename, "Expected 'THEN'.");
      return NULL;
    }

    Token thenTok = parser->currentToken;

    advanceParser(parser); // skip THEN
    
    if (parser->currentToken.type == TOK_EOF) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(thenTok.start, thenTok.end, thenTok.start.filename, "Expected expression.");
      return NULL;
    }

    ASTNode* errHandler = blockParser(parser);

    if (!errHandler) { // Error is already set 
      return NULL;
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_END) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(thenTok.start, thenTok.end, thenTok.start.filename, "Expected 'END'.");
      return NULL;
    }

    advanceParser(parser); // skip END

    return (ASTNode*)initTryCatchNode(tryTok, catchTok, errIdentifier, body, errHandler);
  }

  if (parser->currentToken.type == TOK_RETURN) {
    Token tok = parser->currentToken;

    advanceParser(parser);

    if (parser->currentToken.type == TOK_EOF) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(tok.start, tok.end, tok.start.filename, "Expected expression after 'RETURN'.");
      return NULL;
    }

    ASTNode* expr = andOrParser(parser);

    if (!expr) { // err already set 
      return NULL;
    }

    return (ASTNode*)initReturnNode(tok.start, tok.end, expr);
  }

  if (parser->currentToken.type == TOK_WHILE) {
    Token whileTok = parser->currentToken;
    advanceParser(parser); // skip WHILE token.

    ASTNode* cond = andOrParser(parser);

    if (!cond) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(whileTok.start, whileTok.end, whileTok.start.filename, "Expected expression after 'WHILE'.");
      return NULL;
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_THEN) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(whileTok.start, whileTok.end, whileTok.start.filename, "Expected 'THEN'.");
      return NULL;
    }
    
    Token thenTok = parser->currentToken;
    advanceParser(parser); // skip THEN token.
    
    ASTNode* body = blockParser(parser);

    if (!body) {
      return NULL;
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_END) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(thenTok.start, thenTok.end, thenTok.start.filename, "Expected 'END'.");
      return NULL;
    }

    advanceParser(parser); // skip END token.
    return (ASTNode*)initWhileNode(cond, body, whileTok.start, whileTok.end);
  }

  if (parser->currentToken.type == TOK_FN) {
    Token fnTok = parser->currentToken; // safe copy for error reporting
    
    advanceParser(parser); // skip FN token 

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_IDENTIFIER) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(fnTok.start, fnTok.end, fnTok.start.filename, "Expected function name after 'FN' keyword.");
      return NULL;
    }
    
    char* funcName = stringDup(parser->currentToken.val.s);
    Token fnNameTok = parser->currentToken;

    if (!funcName) {
      return NULL;
    }

    advanceParser(parser); // skip function name 

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_LPAREN) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(fnNameTok.start, fnNameTok.end, fnNameTok.start.filename, "Expected '(' after function name.");
      return NULL;
    }

    advanceParser(parser); // skip '('
    
    size_t paramCount = 0;
    size_t paramCapacity = 16;

    char **params = arenaAlloc(parseArena, sizeof(char*) * paramCapacity);

    if (!params) {
      return NULL;
    }

    while (parser->currentToken.type && parser->currentToken.type != TOK_RPAREN) {
      Token param = parser->currentToken;

      if (param.type != TOK_IDENTIFIER) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(param.start, param.end, param.start.filename, "Expected parameter name.");

        return NULL;
      }

      char* paramName = stringDup(param.val.s);

      if (!paramName) {
        return NULL;
      }

      if (paramCount >= paramCapacity) {
        size_t oldcap = paramCapacity;
        paramCapacity *= 2;

        void* tmp = arenaRealloc(parseArena, params, oldcap * sizeof(char*), sizeof(char*) * paramCapacity);

        if (!tmp) {
          return NULL;
        }

        params = tmp;
      }

      params[paramCount++] = paramName;

      advanceParser(parser); // skip parameter name

      if (parser->currentToken.type != TOK_EOF && parser->currentToken.type == TOK_COMMA) {
        advanceParser(parser); // skip comma
        continue;
      }

      if (parser->currentToken.type != TOK_EOF && parser->currentToken.type == TOK_RPAREN) {
        break;
      }

      if (*parser->error == NULL)
        *parser->error = initSyntaxError(param.start, param.end, param.start.filename, "Expected ',' or ')' after parameter name.");
    
      return NULL;
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_RPAREN) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(fnNameTok.start, fnNameTok.end, fnNameTok.start.filename, "Expected ')'.");

      return NULL;
    }

    advanceParser(parser); // skip ')'
    
    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_THEN) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(fnNameTok.start, fnNameTok.end, fnNameTok.start.filename, "Expected 'THEN'.");

      return NULL;
    }

    advanceParser(parser); // skip THEN.
    
    ASTNode* body = blockParser(parser);

    if (!body) {
      return NULL;
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_END) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(fnNameTok.start, fnNameTok.end, fnNameTok.start.filename, "Expected 'END'.");
      return NULL;
    }

    advanceParser(parser); // skip END.
  
    FunctionNode* node = initFunctionNode(body, funcName, params, paramCount);
    
    if (!node) {
      return NULL;
    }

    return (ASTNode*)node;
  }

  if (parser->currentToken.type == TOK_IF) {
    Token ifTok = parser->currentToken; // safe copy for error reporting
    advanceParser(parser);

    ASTNode* condition = andOrParser(parser);

    if (!condition) return NULL;

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_THEN) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(ifTok.start, ifTok.end, ifTok.start.filename, "Expected THEN");
      return NULL;
    }

    advanceParser(parser);

    ASTNode* thenExpr = blockParser(parser);

    if (!thenExpr) {
      return NULL;
    }

    size_t size = 0;
    size_t capacity = 8;
    ASTNode** elifConds = arenaAlloc(parseArena, capacity * sizeof(ASTNode*));
    ASTNode** elifExprs = arenaAlloc(parseArena, capacity * sizeof(ASTNode*));

    if (!elifConds || !elifExprs) {
      return NULL;
    }

    while (parser->currentToken.type != TOK_EOF && parser->currentToken.type == TOK_ELIF) {
      Token elifTok = parser->currentToken; // safe copy for error reporting
      advanceParser(parser);

      if (parser->currentToken.type == TOK_EOF) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(elifTok.start, elifTok.end, elifTok.start.filename, "Expected expression after 'ELIF' keyword.");
        return NULL;
      }

      ASTNode* elifCond = andOrParser(parser);

      if (!elifCond) {
        return NULL;
      }

      if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_THEN) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(elifTok.start, elifTok.end, elifTok.start.filename, "Expected THEN after ELIF condition");
        
        return NULL;
      }

      advanceParser(parser);

      ASTNode* elifExpr = blockParser(parser);

      if (!elifExpr) {
        return NULL;
      }

      if (size >= capacity) {
        size_t oldcap = capacity;
        capacity *= 2;
        void* tmp1 = arenaRealloc(parseArena, elifConds, oldcap * sizeof(ASTNode*), sizeof(ASTNode*) * capacity);
        void* tmp2 = arenaRealloc(parseArena, elifExprs, oldcap * sizeof(ASTNode*), sizeof(ASTNode*) * capacity);

        if (!tmp1 || !tmp2) {
          if (tmp1) elifConds = tmp1;
          if (tmp2) elifExprs = tmp2;

          return NULL;
        }

        elifConds = tmp1;
        elifExprs = tmp2;
      }

      elifConds[size] = elifCond;
      elifExprs[size] = elifExpr;
      size++;
    }

    ASTNode* elseExpr = NULL;
    Token tok = {.type = TOK_EOF};

    if (parser->currentToken.type != TOK_EOF && parser->currentToken.type == TOK_ELSE) {
      tok = parser->currentToken; // safe copy

      advanceParser(parser);

      if (parser->currentToken.type == TOK_EOF) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(tok.start, tok.end, tok.start.filename, "Expected expression after 'ELSE' keyword.");
        return NULL;
      }

      elseExpr = blockParser(parser);

      if (!elseExpr) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(tok.start, tok.end, tok.start.filename, "Expected expression after 'ELSE' keyword.");

        return NULL;
      }
    }

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_END) {
      Token endTok = tok.type != TOK_EOF ? tok : ifTok;

      if (*parser->error == NULL) *parser->error = initSyntaxError(ifTok.start, endTok.end, ifTok.start.filename, "Expected 'END' token.");
      
      return NULL;
    }

    advanceParser(parser); // skip END tok

    return (ASTNode*)initIfNode(condition, thenExpr, elifConds, elifExprs, size, elseExpr);
  }
  
  if (parser->currentToken.type == TOK_IMPORT) {
    Token tok = parser->currentToken; // safe copy 
    
    advanceParser(parser); // skip IMPORT 

    if (parser->currentToken.type == TOK_EOF || parser->currentToken.type != TOK_STRING) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(tok.start, tok.end, tok.start.filename, "Expected file path after 'IMPORT' keyword.");

      return NULL;
    }

    Token filePathToken = parser->currentToken;

    advanceParser(parser); // skip file path token 

    return (ASTNode*)initImportNode(filePathToken);
  }

  if (parser->currentToken.type == TOK_VAR) {
    Token tok = parser->currentToken; // safe copy
    advanceParser(parser);

    if (parser->currentToken.type == TOK_EOF) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(tok.start, tok.end, tok.start.filename, "Expression token after 'VAR' keyword.");
      return NULL;
    }

    if (parser->currentToken.type != TOK_IDENTIFIER) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(parser->currentToken.start, parser->currentToken.end, parser->currentToken.start.filename, "Expected identifier after 'VAR'");
      return NULL;
    }

    char *varName = parser->currentToken.val.s;

    tok = parser->currentToken; // update safe copy

    advanceParser(parser);

    if (parser->currentToken.type == TOK_EOF) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(tok.start, tok.end, tok.start.filename, "Missing '=' after identifier");
      return NULL;
    }

    if (parser->currentToken.type != TOK_EQ) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(parser->currentToken.start, parser->currentToken.end, parser->currentToken.start.filename, "Expected '=' after identifier");
      return NULL;
    }

    tok = parser->currentToken; // update safe copy

    advanceParser(parser);

    ASTNode* expr = andOrParser(parser);

    if (!expr) { // Error is already set 
      return NULL;
    }

    return (ASTNode*)initVarAssignNode(varName, expr);
  }

  if (parser->currentToken.type == TOK_IDENTIFIER) {
    ParserCheckpoint checkpoint = saveParser(parser);

    Token identTok = parser->currentToken;
    advanceParser(parser); // skip identifier 

    if (parser->currentToken.type == TOK_EQ) {
      Token eq = parser->currentToken;

      advanceParser(parser); // skip '='
      
      ASTNode* expr = andOrParser(parser);

      if (!expr) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(eq.start, eq.end, eq.start.filename, "Expected expression after '='.");
        return NULL;
      }

      return (ASTNode*)initVarAssignNode(identTok.val.s, expr);
    } 

    if (parser->currentToken.type == TOK_LBRACK) {
      Token lbrack = parser->currentToken;
      advanceParser(parser); // skip '[

      ASTNode* index = andOrParser(parser);

      if (!index) return NULL;

      if (parser->currentToken.type != TOK_RBRACK) {
        if (*parser->error == NULL) *parser->error = initSyntaxError(lbrack.start, lbrack.end, lbrack.start.filename, "Expected ']'.");
        return NULL;
      }

      advanceParser(parser); // skip ']'
      
      if (parser->currentToken.type == TOK_EQ) {
        Token eq = parser->currentToken;
          
        advanceParser(parser); // skip '='
        
        ASTNode* value = andOrParser(parser);

        if (!value) { 
          return NULL; 
        } 

        return (ASTNode*)initIndexAssignNode(identTok, index, value, identTok.start, eq.end);
      }
      
      rewindParser(parser, checkpoint);
    } else {
      rewindParser(parser, checkpoint);
    }
  }

  ASTNode* left = termParser(parser);
  if (!left) return NULL; // error is already set

  while (parser->currentToken.type != TOK_EOF && ((parser->currentToken.type == TOK_PLUS) || (parser->currentToken.type == TOK_MINUS))) {
    Token opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = termParser(parser);

    if (!right) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(opTok.start, opTok.end, opTok.start.filename, "Expression expected");
      return NULL;
    }

    left = (ASTNode*)initBinOpNode(left, opTok, right);

    if (!left) {
      return NULL;
    }
  }

  return left;
}

ASTNode* andOrParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = compExprParser(parser);
  if (!left) return NULL;

  while (parser->currentToken.type != TOK_EOF && (parser->currentToken.type == TOK_AND || parser->currentToken.type == TOK_OR)) {
    Token opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = compExprParser(parser);

    if (!right) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(opTok.start, opTok.end, opTok.start.filename, "Expression expected after logical operator");
      return NULL;
    }

    left = (ASTNode*)initBinOpNode(left, opTok, right);

    if (!left) {
      return NULL;
    }
  }

  return left;
}

ASTNode* compExprParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* left = exprParser(parser);
  if (!left) return NULL;

  while (parser->currentToken.type != TOK_EOF && ((parser->currentToken.type == TOK_EE) ||
         (parser->currentToken.type == TOK_NE) ||
         (parser->currentToken.type == TOK_LT) ||
         (parser->currentToken.type == TOK_GT) ||
         (parser->currentToken.type == TOK_LTE) ||
         (parser->currentToken.type == TOK_GTE))) {

    Token opTok = parser->currentToken;
    advanceParser(parser);

    ASTNode* right = exprParser(parser);

    if (!right) {
      if (*parser->error == NULL) *parser->error = initSyntaxError(opTok.start, opTok.end, opTok.start.filename, "Expression expected after comparison operator");
      return NULL;
    }

    left = (ASTNode*)initBinOpNode(left, opTok, right);

    if (!left) {
      return NULL;
    }
  }

  return left;
}

ASTNode* parseParser(Parser* parser) {
  if (!parser) return NULL;

  ASTNode* res = andOrParser(parser);

  if (!res) return NULL; // error is already set

  if (parser->currentToken.type != TOK_EOF) {
    if (*parser->error == NULL) *parser->error = initSyntaxError(parser->currentToken.start, parser->currentToken.end, parser->currentToken.start.filename, "Unexpected token after expression");
    return NULL;
  }

  return res;
}

ASTNode* parseProgram(Parser* parser) {
  if (!parser) return NULL;

  size_t size = 0;
  size_t capacity = 1024;

  ASTNode **statements = arenaAlloc(parseArena, capacity * sizeof(ASTNode*));

  while (parser->currentToken.type != TOK_EOF) {
    ASTNode *statement = andOrParser(parser);

    if (!statement) {
      return NULL;
    } 

    if (size >= capacity) {
      size_t oldcap = capacity;
      capacity *= 2;

      void *tmp = arenaRealloc(parseArena, statements, oldcap * sizeof(ASTNode*), sizeof(ASTNode*) * capacity);

      if (!tmp) {
        return NULL;
      }

      statements = tmp;
    }

    statements[size++] = statement;
  }

  statements[size] = NULL;

  return (ASTNode*)initProgramNode(statements, size);
}
