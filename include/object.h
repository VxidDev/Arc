#ifndef OBJECT_H
#define OBJECT_H

#include "error.h"
#include "node.h"

#include "lexer.h"
#include "parser.h"
#include "token.h"

typedef struct SymbolTable SymbolTable;

typedef enum ObjType {
  OBJ_NUMBER_INT,
  OBJ_NUMBER_FLOAT,
  OBJ_STRING,
  OBJ_LIST,
  OBJ_FUNCTION,
  OBJ_NATIVE_FUNCTION,
  OBJ_FUNCTION_CALL,
  OBJ_MODULE,
  OBJ_ERROR,
  OBJ_RETURN
} ObjType;


typedef struct Object {
  ObjType type;
} Object;

typedef Object* (*NativeFunc)(Object** argc, size_t argCount);
typedef void (*NativeModuleInit)(SymbolTable* globals);

typedef struct {
  const char* name;
  NativeModuleInit init;
} NativeModuleEntry;

extern const NativeModuleEntry* stdlibModules[];

typedef struct Number {
  Object base;

  union {
    long i;
    double f;
  } as;

} Number;

typedef struct String {
  Object base;
  char *value;
  uint64_t len;
} String;

typedef struct List {
  Object base;
  Object** objects;
  uint64_t size, capacity;
} List;

typedef struct Function {
  Object base;

  char *name;

  char **params;
  size_t paramCount;

  ASTNode* body;
} Function;

typedef struct FunctionCall {
  Object base;

  Function *function;
  Object **args;
  size_t argCount;

  SymbolTable *env;
} FunctionCall;

typedef struct NativeFunction {
  Object base;

  char *name;
  NativeFunc function;

  bool isVariadic;

  size_t requiredArgCount;
} NativeFunction;

typedef struct Module {
  Object base;

  ASTNode* astTree;
  Lexer* lexer;
  Parser* parser;
  Token** tokens;
  size_t tokenAmount;
  char *fileContent;
} Module;

typedef struct ProgramError {
  Object base;

  char* details;
} ProgramError;

typedef struct Return {
  Object base;
  Object* value;
} Return;

typedef struct EvalResultNumber {
  Number* num;
  ErrType err;
} EvalResultNumber;

Number* initInt(long value);
Number* initFloat(double value);
Number* copyNumber(Number *num);

String* initString(char *value, uint64_t len);
String* copyString(String *str);

List* initList(Object** list, uint64_t size, uint64_t capacity);
List* copyList(List* list);

Module* initModule(ASTNode* astTree, Lexer* lexer, Parser* parser, char *fileContent, Token** tokens, size_t tokenAmount);

Function* initFunction(FunctionNode* node);
NativeFunction* initNativeFunction(char *name, NativeFunc func, size_t requiredArgCount, bool isVariadic);
Function* copyFunction(Function* func);
NativeFunction* copyNativeFunction(NativeFunction* func);
ProgramError* initProgramError(char *details);
Return* initReturn(Object* value);

FunctionCall* initFunctionCall(FunctionCallNode* node, Object* calleeObj, SymbolTable* parentEnv, char *filename, Error** err);

Object* copyObject(Object *obj);

ErrType addNumber(Number* dest, const Number* src);
ErrType subNumber(Number* dest, const Number* src);
ErrType mulNumber(Number* dest, const Number* src);
ErrType divNumber(Number* dest, const Number* src);
ErrType powNumber(Number* dest, const Number* src);

ErrType isEqualNumber(Number* dest, const Number* src);
ErrType isNotEqualNumber(Number* dest, const Number* src);
ErrType isLessThanEqualNumber(Number* dest, const Number* src);
ErrType isGreaterThanEqualNumber(Number* dest, const Number* src);
ErrType isLessThanNumber(Number* dest, const Number* src);
ErrType isGreaterThanNumber(Number* dest, const Number* src);

ErrType andNumber(Number* dest, const Number* src);
ErrType orNumber(Number* dest, const Number* src);

String* addString(const String* dest, const String* src);
String* mulString(const String* dest, const Number* src);

void freeObject(Object* obj);
#endif // OBJECT_H
