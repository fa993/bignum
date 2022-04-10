#ifndef FA993_BIGNUM_H
#define FA993_BIGNUM_H

typedef unsigned long mytype;
typedef struct bigint bigint;

bigint* create(mytype num);
int equals(bigint *a1, bigint *a2);
void add(bigint* a1, bigint* a2);
void printBinaryToString(char* buffer, bigint* a1);
void printHexToString(char* buffer, bigint* a1);
void printDecimalToString(char* buffer, bigint* a1);
void printBaseToString(char* buffer, bigint* a1, int b, char *encoding);
void destroy(bigint* a1);

#endif
