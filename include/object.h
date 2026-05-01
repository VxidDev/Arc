#ifndef OBJECT_H
#define OBJECT_H

#include "error.h"

typedef enum ObjType {
  OBJ_NUMBER_INT,
  OBJ_NUMBER_FLOAT,
} ObjType;

typedef struct Object {
  ObjType type;
} Object;

typedef struct Number {
  Object base;

  union {
    long i;
    double f;
  } as;

} Number;

typedef struct EvalResultNumber {
  Number* num;
  ErrType err;
} EvalResultNumber;

Number* initInt(long value);
Number* initFloat(double value);
Number *copyNumber(Number *num);

EvalResultNumber addNumber(const Number* dest, const Number* src);
EvalResultNumber subNumber(const Number* dest, const Number* src);
EvalResultNumber mulNumber(const Number* dest, const Number* src);
EvalResultNumber divNumber(const Number* dest, const Number* src);
EvalResultNumber powNumber(const Number* dest, const Number* src);

#endif // OBJECT_H
