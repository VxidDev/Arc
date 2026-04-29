#include "../../include/object.h"

#include <stdlib.h>

static inline double toDouble(const Number* n) {
  return (n->base.type == OBJ_NUMBER_FLOAT)
    ? n->as.f
    : (double)n->as.i;
}

static inline long toLong(const Number* n) {
  return (n->base.type == OBJ_NUMBER_INT)
    ? n->as.i
    : (long)n->as.f;
}

static inline ObjType promote(const Number* a, const Number* b) {
  return (a->base.type == OBJ_NUMBER_FLOAT ||
          b->base.type == OBJ_NUMBER_FLOAT)
    ? OBJ_NUMBER_FLOAT
    : OBJ_NUMBER_INT;
}

Number *initInt(long value) {
  Number* number = malloc(sizeof(Number));

  if (!number) return NULL;

  number->as.i = value;
  number->base.type = OBJ_NUMBER_INT;

  return number;
}

Number *initFloat(double value) {
  Number* number = malloc(sizeof(Number));

  if (!number) return NULL;

  number->as.f = value;
  number->base.type = OBJ_NUMBER_FLOAT;

  return number;
}

Number* addNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    double result = toDouble(src) + toDouble(dest);
    return initFloat(result);
  }

  long result = toLong(src) + toLong(dest);
  return initInt(result);
}

Number* subNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    double result = toDouble(src) - toDouble(dest);
    return initFloat(result);
  }

  long result = toLong(src) - toLong(dest);
  return initInt(result);
}

Number* divNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    double destVal = toDouble(dest);
    double result = destVal == 0 ? -1 : toDouble(src) / destVal;
    return initFloat(result);
  }
  
  long destVal = toLong(dest);
  long result = destVal == 0 ? -1 : toLong(src) / destVal;
  return initInt(result);
}

Number* mulNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return NULL;

  ObjType type = promote(dest, src);

  if (type == OBJ_NUMBER_FLOAT) {
    double result = toDouble(src) * toDouble(dest);
    return initFloat(result);
  }

  long result = toLong(src) * toLong(dest);
  return initInt(result);
}


