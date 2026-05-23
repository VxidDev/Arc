#include "../../include/repl/printast.h"
#include "../../include/repl/repl.h"
#include "../../include/node.h"
#include "../../include/ansi-colors.h"

#include <stdio.h>
#include <string.h>

#include <inttypes.h>

void printAST(ASTNode* node) {
  if (!node) return;

  switch (node->type) {
    case NODE_NUMBER: {
      NumberNode* n = (NumberNode*)node;

      if (n->token->type == TOK_INT) {
        printf("%s%" PRId64 "%s", COLOR(ANSI_BRIGHT_YELLOW_FG), n->token->val.i, COLOR(ANSI_RESET));
      } else if (n->token->type == TOK_FLOAT) {
        printf("%s%s%.*f%s", COLOR(ANSI_DIM), COLOR(ANSI_YELLOW_FG), _FLOAT_PRECISION, n->token->val.f, COLOR(ANSI_RESET));
      }

      break;
    }

    case NODE_BINOP: {
      BinOpNode* b = (BinOpNode*)node;

      putchar('(');
      printAST(b->leftNode);

      printf(" %s%s%s ", COLOR(ANSI_BRIGHT_CYAN_FG), "BINOP", COLOR(ANSI_RESET));

      printAST(b->rightNode);
      putchar(')');
      break;
    }

    case NODE_UNARYOP: {
      UnaryOpNode* u = (UnaryOpNode*)node;

      printf("(%s%s%s ", COLOR(ANSI_BRIGHT_BLACK_FG), "UNARYOP", COLOR(ANSI_RESET));
      printAST(u->node);
      putchar(')');

      break;
    }

    case NODE_VARASSIGN: {
      VarAssignNode* va = (VarAssignNode*)node;

      printf("%s[%s = ", COLOR(ANSI_BRIGHT_MAGENTA_FG), va->identifier);

      printAST(va->value);

      printf("]%s", COLOR(ANSI_RESET));
      break;
    }

    case NODE_VARACCESS: {
      VarAccessNode* va = (VarAccessNode*)node;
      printf("%s[VAR-ACCESS:%s]%s", COLOR(ANSI_BRIGHT_MAGENTA_FG), va->token->val.s, COLOR(ANSI_RESET));
      break;
    }

    case NODE_STRING: {
      StringNode* str = (StringNode*)node;
      printf("%sSTRING:\"%s\"%s", COLOR(ANSI_BRIGHT_GREEN_FG), str->token->val.s, COLOR(ANSI_RESET));
      break;
    }

    case NODE_IF: {
      IfNode* n = (IfNode*)node;
      printf("%sIF:%s", COLOR(ANSI_BRIGHT_CYAN_FG), COLOR(ANSI_RESET));
      printAST(n->condition);
      printf("%s THEN:%s", COLOR(ANSI_CYAN_FG), COLOR(ANSI_RESET));
      printAST(n->thenExpr);

      for (size_t i = 0; i < n->elifCount; i++) {
        printf("%s ELIF:%s", COLOR(ANSI_CYAN_FG), COLOR(ANSI_RESET));
        printAST(n->elifConds[i]);
        printf("%s THEN:%s", COLOR(ANSI_CYAN_FG), COLOR(ANSI_RESET));
        printAST(n->elifExprs[i]);
      }

      if (n->elseExpr) {
        printf("%s ELSE:%s", COLOR(ANSI_BRIGHT_CYAN_FG), COLOR(ANSI_RESET));
        printAST(n->elseExpr);
      }

      break;
    }

    case NODE_PROGRAM: {
      ProgramNode* p = (ProgramNode*)node;
      for (size_t i = 0; i < p->count; i++) {
        printAST(p->statements[i]);
        putchar('\n');
      }
      break;
    }

    case NODE_LIST: {
      ListNode* list = (ListNode*)node;

      putchar('[');

      for (uint64_t i = 0; i < list->size;) {
        ASTNode* node = list->objects[i];
        printAST(node);
        if (++i < list->size) printf(" , ");
      }

      putchar(']');
      break;
    }

    case NODE_INDEX: {
      IndexNode* idx = (IndexNode*)node;

      printf("%sIndexAccess[", COLOR(ANSI_MAGENTA_FG));
      printAST(idx->target);
      printf(" , ");
      printAST(idx->index);
      printf("]%s\n", COLOR(ANSI_RESET));

      break;
    }
  }
}

