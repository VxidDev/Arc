#include "../../include/object.h"

#include <stdlib.h>

Number* initNumber(long value) {
  Number* number = malloc(sizeof(Number));

  if (!number) return NULL;

  number->value = value;
  return number;
}

Number* addNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return NULL;

  Number* out = malloc(sizeof(Number));

  if (!out) return NULL;

  out->value = src->value + dest->value;
  return out;
}

Number* subNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return NULL;

  Number* out = malloc(sizeof(Number));

  if (!out) return NULL;

  out->value = src->value - dest->value;
  return out;
}

Number* divNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return NULL;

  Number* out = malloc(sizeof(Number));

  if (!out) return NULL;
  
  out->value = !dest->value ? -1 : src->value / dest->value;
  return out;
}

Number* mulNumber(const Number* dest, const Number* src) {
  if (!dest || !src) return NULL;

  Number* out = malloc(sizeof(Number));

  if (!out) return NULL;

  out->value = src->value * dest->value;
  out->base.type = OBJ_NUMBER;
  return out;
}


