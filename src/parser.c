#include "../include/parser.h"
#include "../include/token.h"
#include "../include/utils.h"

#include "../include/memarena.h"

#include <stdlib.h>
#include <string.h>

#define MAX_DEPTH 8192

static int sDepth = 0;

static inline void setError(Parser* parser, Position start, Position end, char* msg) {
  if (*parser->error == NULL)
    *parser->error = initSyntaxError(start, end, parser->lexer->filename, msg, parser->lexer->text);
}

Parser* initParser(Lexer* lexer, Error **error) {
  if (!error) return NULL;

  Parser* parser = arenaAlloc(parseArena, sizeof(Parser));

  if (!parser) return NULL;

  parser->lexer = lexer;
  parser->currentToken = (Token){ .type = TOK_EOF };

  parser->error = error;

  advanceParser(parser);

  return parser;
}

Token advanceParser(Parser* parser) {
  if (!parser) return (Token){.type = TOK_EOF};

  if (parser->currentToken.type != TOK_INVALID) {
    parser->currentToken = lexNextToken(parser->lexer, parser->error);
  } 

  return parser->currentToken;
}

static bool getBinOpInfo(TokType t, int* prec, bool* rightAssoc) {
  switch (t) {
    case TOK_OR: case TOK_AND:
      *prec = 1; *rightAssoc = false; return true;
    case TOK_EE: case TOK_NE: case TOK_LT: case TOK_GT: case TOK_LTE: case TOK_GTE:
      *prec = 2; *rightAssoc = false; return true;
    case TOK_PLUS: case TOK_MINUS:
      *prec = 3; *rightAssoc = false; return true;
    case TOK_MUL: case TOK_DIV:
      *prec = 4; *rightAssoc = false; return true;
    case TOK_POW:
      *prec = 5; *rightAssoc = true; return true;
    default:
      return false;
  }
}

static ASTNode* parseUnary(Parser* parser);
static ASTNode* parseExprPrimary(Parser* parser);
ASTNode* parseExpr(Parser* parser, int minPrec);
ASTNode* postfixParser(Parser* parser);
ASTNode* exprParser(Parser* parser);
ASTNode* termParser(Parser* parser);
ASTNode* factorParser(Parser* parser);
ASTNode* postfixParser(Parser* parser);
ASTNode* blockParser(Parser* parser);
static ASTNode* parseClass(Parser* parser);
static ASTNode* parseUnary(Parser* parser);
static ASTNode* parseWhile(Parser* parser);
static ASTNode* parseIdentifier(Parser* parser);
static ASTNode* parseTryCatch(Parser* parser);
static ASTNode* parseReturn(Parser* parser);
static ASTNode* parseFunction(Parser* parser);
static ASTNode* parseFor(Parser* parser);
static ASTNode* parseVar(Parser* parser);
static ASTNode* parseImport(Parser* parser);
static ASTNode* parseIf(Parser* parser);

static ASTNode* continueExpr(Parser* parser, ASTNode* left, int minPrec) {
  for (;;) {
    int prec;
    bool rightAssoc;
    
    if (!getBinOpInfo(parser->currentToken.type, &prec, &rightAssoc)) break;
    if (prec < minPrec) break;

    Token opTok = parser->currentToken;
    advanceParser(parser);

    int nextMinPrec = rightAssoc ? prec : prec + 1;
    ASTNode* right = parseExpr(parser, nextMinPrec);

    if (!right) {
      setError(parser, opTok.start, opTok.end, "Expression expected after operator");
      return NULL;
    }

    left = (ASTNode*)initBinOpNode(left, opTok, right);
    if (!left) return NULL;
  }

  return left;
}

static ASTNode* parseExprPrimary(Parser* parser) {
  switch (parser->currentToken.type) {
    case TOK_RPAREN:
      setError(parser, parser->currentToken.start, parser->currentToken.end, "Unexpected ')'");
      return NULL;

    case TOK_EOF: return NULL; // TODO: error handling
    case TOK_CLASS: return parseClass(parser);
    case TOK_FOR:   return parseFor(parser);

    case TOK_BREAK: {
      ASTNode* node = (ASTNode*)initBreakNode(parser->currentToken);
      advanceParser(parser);
      return node;
    }

    case TOK_CONTINUE: {
      ASTNode* node = (ASTNode*)initContinueNode(parser->currentToken);
      advanceParser(parser);
      return node;
    }

    case TOK_TRY:    return parseTryCatch(parser);
    case TOK_RETURN: return parseReturn(parser);
    case TOK_WHILE:  return parseWhile(parser);
    case TOK_FN:     return parseFunction(parser);
    case TOK_IF:     return parseIf(parser);
    case TOK_IMPORT: return parseImport(parser);
    case TOK_VAR:    return parseVar(parser);
    case TOK_IDENTIFIER: return parseIdentifier(parser);

    default: return parseUnary(parser);
  }
}

ASTNode* parseExpr(Parser* parser, int minPrec) {
  if (++sDepth > MAX_DEPTH) {
    --sDepth;
    setError(parser, parser->currentToken.start, parser->currentToken.end, "Expression too deeply nested");
    return NULL;
  }

  ASTNode* left = parseExprPrimary(parser);
  ASTNode* result = left ? continueExpr(parser, left, minPrec) : NULL;

  --sDepth;
  return result;
}

static ASTNode* parseUnary(Parser* parser) {
  Token token = parser->currentToken;

  if (token.type == TOK_PLUS || token.type == TOK_MINUS || token.type == TOK_NOT) {
    advanceParser(parser);
    ASTNode* operand = parseUnary(parser);
    
    if (!operand) {
      setError(parser, token.start, token.end, "Expression expected");
      return NULL;
    }
    
    return (ASTNode*)initUnaryOpNode(token, operand);
  }

  return postfixParser(parser);
}


ASTNode* atomParser(Parser* parser) {
  if (!parser || parser->currentToken.type == TOK_EOF) return NULL;

  Token token = parser->currentToken;

  if (token.type == TOK_STRING) {
    advanceParser(parser);
    return (ASTNode*)initStringNode(token);
  }

  if (token.type == TOK_NULL) {
    advanceParser(parser);
    return (ASTNode*)initNullNode(token);
  }

  if (token.type == TOK_TRUE || token.type == TOK_FALSE) {
    advanceParser(parser);

    Token numTok = token;
    numTok.type = TOK_INT;
    numTok.val.i = (token.type == TOK_TRUE) ? 1 : 0;

    return (ASTNode*)initNumberNode(numTok);
  }

  if ((token.type == TOK_INT) || (token.type == TOK_FLOAT)) {
    advanceParser(parser);
    return (ASTNode*)initNumberNode(token);
  } else if (token.type == TOK_IDENTIFIER) {
    advanceParser(parser);
    return (ASTNode*)initVarAccessNode(token);
  } else if (token.type == TOK_LPAREN) {
    advanceParser(parser);

    Position start = token.start; // safe copy
    Position end = token.end;

    ASTNode* expr = parseExpr(parser, 0);

    if (!expr) {
      setError(parser, start, end, "Expression expected");
      return NULL;
    }

    if (parser->currentToken.type == TOK_RPAREN) {
      advanceParser(parser);
      return expr;
    } else {
      setError(parser, token.start, token.end, "Expression expected");
      return NULL;
    }
  }

  if (token.type == TOK_LBRACK) {
    Token start = parser->currentToken;

    advanceParser(parser);
    
    if (parser->currentToken.type == TOK_EOF) {
      setError(parser, start.start, start.end, "Unexpected EOF.");
      return NULL;
    }

    uint64_t size = 0;
    uint64_t capacity = 64;

    ASTNode** objects = arenaAlloc(parseArena, capacity * sizeof(ASTNode*));

    if (!objects) {
      return NULL;
    }

    while (parser->currentToken.type != TOK_RBRACK) {
      token = parser->currentToken;
      ASTNode* val = parseExpr(parser, 0);

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

      if (parser->currentToken.type != TOK_RBRACK) {
        setError(parser, parser->currentToken.start, parser->currentToken.end, "Expected ',' or ']'.");
        return NULL;
      }
    }

    if (parser->currentToken.type != TOK_RBRACK) {
      setError(parser, start.start, start.end, "Unterminated list: expected ']'.");

      return NULL;
    }

    Token end = parser->currentToken;

    advanceParser(parser);

    objects[size] = NULL;

    return (ASTNode*)initListNode(start, end, objects, size, capacity);
  }

  setError(parser, token.start, token.end, "Expression expected");
  return NULL;
}

ASTNode* __blockParser(Parser* parser);
ASTNode* __exprParser(Parser* parser);

ASTNode* blockParser(Parser* parser) {
  if (++sDepth > MAX_DEPTH) {
    --sDepth;

    if (parser && *parser->error == NULL)
      setError(parser, parser->currentToken.start, parser->currentToken.end, "Block too deeply nested");

    return NULL;
  }

  ASTNode* result = __blockParser(parser);
  --sDepth;

  return result;
}

ASTNode* __blockParser(Parser* parser) {
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
    ASTNode* stmt = parseExpr(parser, 0);
  
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

  for (;;) {
    if (parser->currentToken.type == TOK_LPAREN) {
      Position start = parser->currentToken.start;
      Position end = parser->currentToken.end;

      advanceParser(parser); // skip '('

      end = parser->currentToken.end;

      size_t size = 0;
      size_t capacity = 16;

      ASTNode** args = arenaAlloc(parseArena, sizeof(ASTNode*) * capacity);
      if (!args) return NULL;

      if (parser->currentToken.type == TOK_RPAREN) {
        advanceParser(parser);
        node = (ASTNode*)initFunctionCallNode(node, args, 0, start, end);
        continue;
      }

      while (parser->currentToken.type != TOK_RPAREN) {
        end = parser->currentToken.end;
        ASTNode* arg = parseExpr(parser, 0);

        if (!arg) return NULL;

        if (size >= capacity) {
          size_t oldcap = capacity;
          capacity *= 2;
          args = arenaRealloc(parseArena, args, oldcap * sizeof(ASTNode*), sizeof(ASTNode*) * capacity);
          if (!args) return NULL;
        }

        args[size++] = arg;

        if (parser->currentToken.type == TOK_COMMA) {
          end = parser->currentToken.end;
          advanceParser(parser);
        }
      }

      if (parser->currentToken.type != TOK_RPAREN) {
        setError(parser, start, end, "Expected ')'.");
        return NULL;
      }

      advanceParser(parser);

      node = (ASTNode*)initFunctionCallNode(node, args, size, start, end);
      continue;
    }

    if (parser->currentToken.type == TOK_LBRACK) {
      Position start = parser->currentToken.start;
      Position end = parser->currentToken.end;

      advanceParser(parser); // skip '['

      if (parser->currentToken.type == TOK_EOF) {
        setError(parser, start, end, "Expression expected.");
        return NULL;
      }

      end = parser->currentToken.end;

      ASTNode* index = parseExpr(parser, 0);
      if (!index) return NULL;

      if (parser->currentToken.type != TOK_RBRACK) {
        setError(parser, start, end, "Expected ']'");
        return NULL;
      }

      end = parser->currentToken.end;

      advanceParser(parser); // skip ']'

      node = (ASTNode*)initIndexNode(node, index, start, end);
      continue;
    }

    if (parser->currentToken.type == TOK_DOT) {
      Position start = parser->currentToken.start;

      advanceParser(parser); // skip '.'

      if (parser->currentToken.type != TOK_IDENTIFIER) {
        setError(parser, start, parser->currentToken.end, "Expected identifier after '.'.");
        return NULL;
      }

      Token field = parser->currentToken;
      Position end = field.end;

      advanceParser(parser); // skip identifier

      node = (ASTNode*)initPropertyAccessNode(node, field, start, end);
      continue;
    }

    break;
  }

  return node;
}

static ASTNode* parseClass(Parser* parser) {
  Token tok = parser->currentToken; // safe copy for errors 
  Position start = tok.start;

  advanceParser(parser);

  if (parser->currentToken.type != TOK_IDENTIFIER) {
    setError(parser, start, tok.end, "Expected identifier after keyword 'CLASS'.");
    return NULL;
  }

  Token identifier = parser->currentToken;
  
  advanceParser(parser);

  if (parser->currentToken.type == TOK_EOF) {
    setError(parser, identifier.start, identifier.end, "Expected expression after identifier.");
    return NULL;
  }

  ASTNode* body = blockParser(parser);

  if (!body) { // error is already set 
    return NULL;
  }

  if (parser->currentToken.type != TOK_END) {
    setError(parser, identifier.start, identifier.end, "Expected 'END'.");
    return NULL;
  }

  advanceParser(parser);

  return (ASTNode*)initClassNode(identifier, body, start, identifier.end);
}

static ASTNode* parseFor(Parser* parser) {
  Token forTok = parser->currentToken;

  Position forStart = forTok.start;
  Position forEnd = forTok.end;

  advanceParser(parser);

  if (parser->currentToken.type != TOK_IDENTIFIER) {
    setError(parser, forStart, forEnd, "Expected identifier after keyword 'FOR'.");
    return NULL;
  }

  Token identTok = parser->currentToken;

  advanceParser(parser);

  if (parser->currentToken.type != TOK_IN) {
    setError(parser, identTok.start, identTok.end, "Expected 'IN' after identifier.");
    return NULL;
  }

  Token inTok = parser->currentToken;
  Position inStart = inTok.start;
  Position inEnd = inTok.end;

  advanceParser(parser);

  if (parser->currentToken.type == TOK_EOF) {
    setError(parser, inStart, inEnd, "Expected expression after 'IN'.");
    return NULL;
  }

  ASTNode* iterable = parseExpr(parser, 0);
  if (!iterable) return NULL;

  if (parser->currentToken.type != TOK_THEN) {
    setError(parser, inStart, inEnd, "Expected 'THEN' after iterable.");
    return NULL;
  }

  Position thenStart = parser->currentToken.start;
  Position thenEnd = parser->currentToken.end;

  advanceParser(parser); // Skip THEN 
  
  if (parser->currentToken.type == TOK_EOF) {
    setError(parser, thenStart, thenEnd, "Expected expression after 'THEN'.");
    return NULL;
  }

  ASTNode* body = blockParser(parser);

  if (!body) { 
    return NULL;
  }

  if (parser->currentToken.type != TOK_END) {
    setError(parser, thenStart, thenEnd, "Expected 'END' after body.");
    return NULL;
  }

  advanceParser(parser); // skip END

  return (ASTNode*)initForNode(forTok, identTok, iterable, body);
}

static ASTNode* parseTryCatch(Parser* parser) {
  Position tryStart = parser->currentToken.start;
  Position tryEnd = parser->currentToken.end;

  advanceParser(parser); // skip TRY 
  
  if (parser->currentToken.type == TOK_EOF) {
    setError(parser, tryStart, tryEnd, "Expected expression.");
    return NULL;
  }

  ASTNode* body = blockParser(parser);

  if (!body) { // Error is already set 
    return NULL;
  }

  if (parser->currentToken.type != TOK_CATCH) {
    setError(parser, tryStart, tryEnd, "Expected 'CATCH'.");
    return NULL;
  }
  
  Position catchStart = parser->currentToken.start;
  Position catchEnd = parser->currentToken.start;

  advanceParser(parser); // Skip CATCH
  
  if (parser->currentToken.type != TOK_IDENTIFIER) {
    setError(parser, catchStart, catchEnd, "Expected identifier after 'CATCH'.");
    return NULL;
  }

  Token errIdentifier = parser->currentToken;

  advanceParser(parser); // skip IDENTIFIER 

  if (parser->currentToken.type != TOK_THEN) {
    setError(parser, catchStart, catchEnd, "Expected 'THEN'.");
    return NULL;
  }

  Position thenStart = parser->currentToken.start;
  Position thenEnd = parser->currentToken.end;

  advanceParser(parser); // skip THEN
  
  if (parser->currentToken.type == TOK_EOF) {
    setError(parser, thenStart, thenEnd, "Expected expression.");
    return NULL;
  }

  ASTNode* errHandler = blockParser(parser);

  if (!errHandler) { // Error is already set 
    return NULL;
  }

  if (parser->currentToken.type != TOK_END) {
    setError(parser, thenStart, thenEnd, "Expected 'END'.");
    return NULL;
  }

  advanceParser(parser); // skip END

  return (ASTNode*)initTryCatchNode(tryStart, catchEnd, errIdentifier, body, errHandler);
}

static ASTNode* parseReturn(Parser* parser) {
  Position start = parser->currentToken.start;
  Position end = parser->currentToken.end;

  advanceParser(parser);

  if (parser->currentToken.type == TOK_EOF) {
    setError(parser, start, end, "Expected expression after 'RETURN'.");
    return NULL;
  }

  ASTNode* expr = parseExpr(parser, 0);

  if (!expr) { // err already set 
    return NULL;
  }

  return (ASTNode*)initReturnNode(start, end, expr);
}

static ASTNode* parseWhile(Parser* parser) {
  Position start = parser->currentToken.start;
  Position end = parser->currentToken.end;

  advanceParser(parser); // skip WHILE token.

  ASTNode* cond = parseExpr(parser, 0);

  if (!cond) {
    setError(parser, start, end, "Expected expression after 'WHILE'.");
    return NULL;
  }

  if (parser->currentToken.type != TOK_THEN) {
    setError(parser, start, end, "Expected 'THEN'.");
    return NULL;
  }
  
  Position thenStart = parser->currentToken.start;
  Position thenEnd = parser->currentToken.end;

  advanceParser(parser); // skip THEN token.
  
  ASTNode* body = blockParser(parser);

  if (!body) {
    return NULL;
  }

  if (parser->currentToken.type != TOK_END) {
    setError(parser, thenStart, thenEnd, "Expected 'END'.");
    return NULL;
  }

  advanceParser(parser); // skip END token.
  return (ASTNode*)initWhileNode(cond, body, start, end);
}

static ASTNode* parseFunction(Parser* parser) {
  Position start = parser->currentToken.start; // safe copy for error reporting
  Position end = parser->currentToken.end;

  advanceParser(parser); // skip FN token 

  if (parser->currentToken.type != TOK_IDENTIFIER) {
    setError(parser, start, end, "Expected function name after 'FN' keyword.");
    return NULL;
  }
  
  char* funcName = parser->currentToken.val.s;
  
  start = parser->currentToken.start;
  end = parser->currentToken.end;

  Position funcStart = start;
  Position funcEnd = end;

  advanceParser(parser); // skip function name 

  if (parser->currentToken.type != TOK_LPAREN) {
    setError(parser, start, end, "Expected '(' after function name.");
    return NULL;
  }

  advanceParser(parser); // skip '('
  
  size_t paramCount = 0;
  size_t paramCapacity = 16;

  char **params = arenaAlloc(parseArena, sizeof(char*) * paramCapacity);

  if (!params) {
    return NULL;
  }

  while (parser->currentToken.type != TOK_RPAREN) {
    Token param = parser->currentToken;

    if (param.type != TOK_IDENTIFIER) {
      setError(parser, param.start, param.end, "Expected parameter name.");
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

    if (parser->currentToken.type == TOK_COMMA) {
      advanceParser(parser); // skip comma
      continue;
    }

    if (parser->currentToken.type == TOK_RPAREN) {
      break;
    }

    setError(parser, param.start, param.end, "Expected ',' or ')' after parameter name.");
    return NULL;
  }

  if (parser->currentToken.type != TOK_RPAREN) {
    setError(parser, start, end, "Expected ')'.");
    return NULL;
  }

  advanceParser(parser); // skip ')'
  
  if (parser->currentToken.type != TOK_THEN) {
    setError(parser, start, end, "Expected 'THEN'.");
    return NULL;
  }

  advanceParser(parser); // skip THEN.
  
  ASTNode* body = blockParser(parser);

  if (!body) {
    return NULL;
  }

  if (parser->currentToken.type != TOK_END) {
    setError(parser, start, end, "Expected 'END'.");
    return NULL;
  }

  advanceParser(parser); // skip END.

  FunctionNode* node = initFunctionNode(body, funcName, params, paramCount, funcStart, funcEnd);
  
  if (!node) {
    return NULL;
  }

  return (ASTNode*)node;
}

static ASTNode* parseIf(Parser* parser) {
  Token ifTok = parser->currentToken; // safe copy for error reporting
  advanceParser(parser);

  ASTNode* condition = parseExpr(parser, 0);

  if (!condition) return NULL;

  if (parser->currentToken.type != TOK_THEN) {
    setError(parser, ifTok.start, ifTok.end, "Expected THEN");
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

  while (parser->currentToken.type == TOK_ELIF) {
    Token elifTok = parser->currentToken; // safe copy for error reporting
    advanceParser(parser);

    if (parser->currentToken.type == TOK_EOF) {
      setError(parser, elifTok.start, elifTok.end, "Expected expression after 'ELIF' keyword.");
      return NULL;
    }

    ASTNode* elifCond = parseExpr(parser, 0);

    if (!elifCond) {
      return NULL;
    }

    if (parser->currentToken.type != TOK_THEN) {
      setError(parser, elifTok.start, elifTok.end, "Expected THEN after ELIF condition");
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

  if (parser->currentToken.type == TOK_ELSE) {
    tok = parser->currentToken; // safe copy

    advanceParser(parser);

    if (parser->currentToken.type == TOK_EOF) {
      setError(parser, tok.start, tok.end, "Expected expression after 'ELSE' keyword.");
      return NULL;
    }

    elseExpr = blockParser(parser);

    if (!elseExpr) {
      setError(parser, tok.start, tok.end, "Expected expression after 'ELSE' keyword.");
      return NULL;
    }
  }

  if (parser->currentToken.type != TOK_END) {
    Token endTok = tok.type != TOK_EOF ? tok : ifTok;
    setError(parser, ifTok.start, endTok.end, "Expected 'END' token.");
    return NULL;
  }

  advanceParser(parser); // skip END tok

  return (ASTNode*)initIfNode(condition, thenExpr, elifConds, elifExprs, size, elseExpr);
}

static ASTNode* parseImport(Parser* parser) {
  Token tok = parser->currentToken; // safe copy 
  
  advanceParser(parser); // skip IMPORT 

  if (parser->currentToken.type != TOK_STRING) {
    setError(parser, tok.start, tok.end, "Expected file path after 'IMPORT' keyword.");
    return NULL;
  }

  Token filePathToken = parser->currentToken;

  advanceParser(parser); // skip file path token 

  return (ASTNode*)initImportNode(filePathToken);
}

static ASTNode* parseVar(Parser* parser) {
  Token tok = parser->currentToken; // safe copy
  Position start = tok.start;
  advanceParser(parser);

  if (parser->currentToken.type == TOK_EOF) {
    setError(parser, tok.start, tok.end, "Expression token after 'VAR' keyword.");
    return NULL;
  }

  if (parser->currentToken.type != TOK_IDENTIFIER) {
    setError(parser, parser->currentToken.start, parser->currentToken.end, "Expected identifier after 'VAR'");
    return NULL;
  }

  char *varName = parser->currentToken.val.s;

  tok = parser->currentToken; // update safe copy

  advanceParser(parser);

  if (parser->currentToken.type == TOK_EOF) {
    setError(parser, tok.start, tok.end, "Missing '=' after identifier");
    return NULL;
  }

  if (parser->currentToken.type != TOK_EQ) {
    setError(parser, parser->currentToken.start, parser->currentToken.end, "Expected '=' after identifier");
    return NULL;
  }

  tok = parser->currentToken; // update safe copy

  advanceParser(parser);

  ASTNode* expr = parseExpr(parser, 0);

  if (!expr) { // Error is already set 
    return NULL;
  }

  return (ASTNode*)initVarAssignNode(varName, expr, start, true);
}

static ASTNode* parseIdentifier(Parser* parser) {
  ASTNode* target = postfixParser(parser);

  if (!target) return NULL;

  if (parser->currentToken.type == TOK_EQ) {
    Token eq = parser->currentToken;

    advanceParser(parser); // skip '='

    ASTNode* value = parseExpr(parser, 0);

    if (!value) {
      setError(parser, eq.start, eq.end, "Expected expression after '='.");
      return NULL;
    }

    if (target->type == NODE_VARACCESS) {
      VarAccessNode* va = (VarAccessNode*)target;
      return (ASTNode*)initVarAssignNode(va->token.val.s, value, va->token.start, false);
    }

    if (target->type == NODE_PROPERTYACCESS) {
      PropertyAccessNode* pa = (PropertyAccessNode*)target;
      return (ASTNode*)initPropertyAssignNode(pa->target, pa->field, value, pa->base.start, eq.end);
    }

    if (target->type == NODE_INDEX) {
      IndexNode* idx = (IndexNode*)target;
      return (ASTNode*)initIndexAssignNode(idx->target, idx->index, value, idx->base.start, eq.end);
    }

    setError(parser, eq.start, eq.end, "Invalid assignment target.");
    return NULL;
  }

  return target; // continueExpr picks up any trailing binary operators
}

ASTNode* parseParser(Parser* parser) { // pretty much dead code
  if (!parser) return NULL;

  ASTNode* res = parseExpr(parser, 0);

  if (!res) return NULL; // error is already set

  if (parser->currentToken.type != TOK_EOF) {
    setError(parser, parser->currentToken.start, parser->currentToken.end, "Unexpected token after expression");
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
    sDepth = 0; // reset per top-level statement
    ASTNode *statement = parseExpr(parser, 0);

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
