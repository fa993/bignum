#ifndef FA993_BIGNUM_H
#define FA993_BIGNUM_H

typedef unsigned long mytype;

struct bigint {

  int size;
  //little endian order
  mytype* parts;
  int digitsPerChunk;

};

extern struct bigint ZERO;
extern struct bigint ONE;

struct bigint add(struct bigint* a1, struct bigint* a2);
void printHexToString(char* buffer, struct bigint* a);
void printBinaryToString(char* buffer, struct bigint* a);
struct bigint convertToBase(struct bigint* a, int base);
void printToString(char* buffer, struct bigint* a);

#endif
