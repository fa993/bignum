#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bignum.h"

mytype zero_val = 0;
struct bigint ZERO = {1, &zero_val};
mytype one_val = 1;
struct bigint ONE = {1, &one_val};

void grow(struct bigint* curr, int newLen) {
  mytype* newParts = calloc(newLen, sizeof(mytype));
  int i;
  for(i = 0; i < curr->size; i++) {
    newParts[i] = curr->parts[i];
  }
  if(curr->size != 0) {
    free(curr->parts);
  }
  curr->parts = newParts;
  curr->size = newLen;
}

int equals(struct bigint* a, struct bigint* b) {
  if(a->size != b->size) {
    return 0;
  }
  int i;
  for(i = 0; i < a->size; i++) {
    if(a->parts[i] != b->parts[i]) {
      return 0;
    }
  }
  return 1;
}

int isZero(struct bigint* a) {
  return equals(&ZERO, a);
}

void chainAddition(struct bigint* op, mytype toAdd, int index) {
  if(index >= op->size) {
    grow(op, index + 1);
  }
  mytype t = op->parts[index];
  op->parts[index] = t + toAdd;
  if(op->parts[index] < t || op->parts[index] < toAdd) {
    chainAddition(op, 1, index + 1);
  }
}

void addToBigInt(struct bigint* op, struct bigint* a1, struct bigint* a2) {
  int sz = 0;
  if(a1->size > sz) {
    sz = a1->size;
  }
  if(a2->size > sz) {
    sz = a2->size;
  }
  int i;
  for(i = 0; i < sz; i++) {
    // op->parts[index] += incrementor;
    if(i < a1->size) {
      // op->parts[index] += a1->parts[index];
      chainAddition(op, a1->parts[i], i);
    }
    if(i < a2->size) {
      // op->parts[index] += a2->parts[index];
      chainAddition(op, a2->parts[i], i);
    }
  }
}

void chainAdditionWithCustomOverflow(struct bigint* op, mytype toAdd, int index, mytype custom_overflow) {
  if(toAdd == 0) {
    return;
  }
  int acc = 0;
  mytype max_val = -1;
  if(index >= op->size) {
    grow(op, index + 1);
  }
  mytype t = op->parts[index];
  op->parts[index] = t + toAdd;
  if(op->parts[index] < t || op->parts[index] < toAdd) {
    op->parts[index] += max_val - custom_overflow + 1;
    acc++;
  }
  if(op->parts[index] > custom_overflow) {
    op->parts[index] %= custom_overflow;
    acc++;
  }
  if(acc != 0) {
    chainAdditionWithCustomOverflow(op, acc, index + 1, custom_overflow);
  }
}

void multiplyByTwo(struct bigint* a, mytype wrapVal) {
  int i;
  int acc = 0;
  int oldAcc = 0;
  mytype elem;
  mytype max_val = -1;
  for(i = 0; i < a->size; i++) {
    elem = a->parts[i];
    if(elem > wrapVal) {
      elem %= wrapVal;
      acc++;
    }
    elem += a->parts[i];
    if(elem < a->parts[i]) {
      //proper overflow not handled well as of now
      elem += max_val - wrapVal + 1;
      acc++;
    }
    if(elem > wrapVal) {
      elem %= wrapVal;
      acc++;
    }
    elem += oldAcc;
    if(elem > wrapVal) {
      elem %= wrapVal;
      acc++;
    }
    a->parts[i] = elem;
    oldAcc = acc;
    acc = 0;
  }
  if(oldAcc != 0) {
    grow(a, i+1);
    a->parts[i] = oldAcc;
  }
}

struct bigint add(struct bigint* a1, struct bigint* a2) {
  struct bigint a3;
  a3.size = 0;
  addToBigInt(&a3, a1, a2);
  return a3;
}

unsigned long init_hex_fmt = 0;

void printHexToString(char* buffer, struct bigint* a) {
  static char fmt_for_hex[50];
  if(init_hex_fmt == 0) {
    init_hex_fmt = 2 * sizeof(mytype);
    sprintf(fmt_for_hex, "%s%lu%s", "%0", init_hex_fmt, "lx");
  }

  int i;
  int j = 0;
  for(i = a->size - 1; i > -1; i--) {
    sprintf(buffer + j, fmt_for_hex, a->parts[i]);
    j += init_hex_fmt;
  }
}

void printBinaryToString(char* buffer, struct bigint* a) {
  int i,j,k;
  mytype marker = -1;
  marker = marker ^ (marker >> 1);
  mytype markerOri = marker;
  mytype t;

  j = 0;
  for(i = a->size - 1; i > -1; i--) {
    marker = markerOri;
    t = a->parts[i];
    do {
      sprintf(buffer + j++, "%d", ((marker & t) > 0) ? 1 : 0);
      marker = marker >> 1;
    } while (marker > 0);
  }
}

struct bigint convertToBase(struct bigint* a, int base) {
  mytype largest_num_for_base = 1;
  mytype test_num = base;
  int power = 0;
  while(test_num >= largest_num_for_base) {
    largest_num_for_base = test_num;
    test_num *= base;
    power++;
  }

  struct bigint transformed;
  transformed.digitsPerChunk = power;
  transformed.size = 0;
  int i,j,k;
  mytype marker = -1;
  marker = marker ^ (marker >> 1);
  mytype markerOri = marker;
  mytype t;

  for(i = a->size - 1; i > -1; i--) {
    if(transformed.size < i + 1) {
      grow(&transformed, i + 1);
    }
    marker = markerOri;
    t = a->parts[i];
    do {
      multiplyByTwo(&transformed, largest_num_for_base);
      chainAdditionWithCustomOverflow(&transformed, ((marker & t) > 0), 0,  largest_num_for_base);
      marker = marker >> 1;
    } while (marker > 0);
  }
  return transformed;
}

void printToString(char* buffer, struct bigint* a) {
  if(a->digitsPerChunk == 0) {
    printBinaryToString(buffer, a);
  } else {
    int i;
    char fmt_for_base[50];
    sprintf(fmt_for_base, "%s%d%s", "%0", a->digitsPerChunk, "lu");

    int j = 0;
    for(i = a->size - 1; i > -1; i--) {
      sprintf(buffer + j, fmt_for_base, a->parts[i]);
      j += a->digitsPerChunk;
    }
  }
}
