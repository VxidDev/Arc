#ifndef REPL_H
#define REPL_H

extern char *_CODE;
extern bool _DEBUG;
extern int _FLOAT_PRECISION;
extern bool _IS_COLORED;

#define COLOR(c) (_IS_COLORED ? (c) : "")

#endif // REPL_H 
