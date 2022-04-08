#ifndef FA993_BIGNUM_H
#define FA993_BIGNUM_H

typedef unsigned long mytype;
typedef struct bigint bigint;

typedef enum base {
  BASE_2, BASE_10, BASE_16
} base;

bigint* create(mytype num);
int equals(bigint *a1, bigint *a2);
void add(bigint* a1, bigint* a2);
void printToString(char* buffer, bigint* a1, base b);
void destroy(bigint* a1);

#endif
