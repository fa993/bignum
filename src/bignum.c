#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bignum.h"

#define TRUE 1
#define FALSE 0

#define MIN(x, y) (x) < (y) ? (x) : (y)
#define MAX(x, y) (x) > (y) ? (x) : (y)

unsigned long init_hex_fmt = 0;

struct bigint {

  int size;
  mytype* parts;

};

bigint* create(mytype num) {
  bigint* cr = calloc(1, sizeof(bigint));
  mytype* t = calloc(1, sizeof(mytype));
  *t = num;
  cr->size = 1;
  cr->parts = t;
  return cr;
}

int equals(bigint* a, bigint* b) {

  /* ignore leading zeros*/

  int i;
  int len1 = MIN(a->size, b->size);
  int len2 = MAX(a->size, b->size);
  for(i = 0; i < len1; i++) {
    if(a->parts[i] != b->parts[i]) {
      return FALSE;
    }
  }
  for(;i < len2;i++) {
    if(i < a->size && a->parts[i] != 0) {
      return FALSE;
    }
    if(i < b->size && b->parts[i] != 0) {
      return FALSE;
    }
  }
  return TRUE;
}

void destroy(bigint* a1) {
  int i;
  free(a1->parts);
  free(a1);
}

void grow(bigint* curr, int newLen) {
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

void chainAddition(bigint* op, mytype toAdd, int index) {
  mytype t;
  if(index >= op->size) {
    grow(op, index + 1);
  }
  t = op->parts[index];
  op->parts[index] = t + toAdd;
  if(op->parts[index] < t || op->parts[index] < toAdd) {
    chainAddition(op, 1, index + 1);
  }
}

void add(bigint* a1, bigint* a2) {
  int sz;
  int i;
  sz = MAX(a1->size, a2->size);
  for(i = 0; i < sz; i++) {
    if(i < a2->size) {
      chainAddition(a1, a2->parts[i], i);
    }
  }
}

int getTotalDigitEstimate(bigint* a1, int base){
  /* TODO: Fix this */
  int digits;
  mytype largest_num_for_base, test_num;
  largest_num_for_base = 1;
  test_num = base;
  digits = 0;
  while(test_num >= largest_num_for_base) {
    largest_num_for_base = test_num;
    test_num *= base;
    digits++;
  }
  return digits * a1->size;
}

void chainAdditionWithCustomOverflow(bigint* op, mytype toAdd, int index, mytype custom_overflow) {
  int acc;
  mytype max_val, t;
  if(toAdd == 0) {
    return;
  }
  acc = 0;
  max_val = -1;
  if(index >= op->size) {
    grow(op, index + 1);
  }
  t = op->parts[index];
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

void multiplyByTwo(bigint* a, mytype wrapVal) {
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

void convertToBase(bigint *to, bigint* a, mytype largest_num_for_base) {
  int i,j,k;
  mytype marker, markerOri, t;
  to->size = 0;
  marker = -1;
  marker = marker ^ (marker >> 1);
  markerOri = marker;
  for(i = a->size - 1; i > -1; i--) {
    if(to->size < i + 1) {
      grow(to, i + 1);
    }
    marker = markerOri;
    t = a->parts[i];
    do {
      multiplyByTwo(to, largest_num_for_base);
      chainAdditionWithCustomOverflow(to, ((marker & t) > 0), 0,  largest_num_for_base);
      marker = marker >> 1;
    } while (marker > 0);
  }
}

void printBinaryToString(char* buffer, bigint* a) {
  int i,j,k;
  mytype marker, markerOri, t;
  marker = -1;
  marker = marker ^ (marker >> 1);
  markerOri = marker;

  j = 0;
  for(i = a->size - 1; i > -1; i--) {
    marker = markerOri;
    t = a->parts[i];
    if(i == a->size - 1) {
      int r = 0;
      do {
        int y = ((marker & t) > 0) ? 1 : 0;
        if(y == 1) {
          r = 1;
        }
        if(r == 1){
          sprintf(buffer + j++, "%d", y);
        }
        marker = marker >> 1;
      } while (marker > 0);
    } else {
      do {
        sprintf(buffer + j++, "%d", ((marker & t) > 0) ? 1 : 0);
        marker = marker >> 1;
      } while (marker > 0);
    }
  }
}

void printHexToString(char* buffer, bigint* a) {
  int i, j;
  static char fmt_for_hex[50];
  if(init_hex_fmt == 0) {
    init_hex_fmt = 2 * sizeof(mytype);
    sprintf(fmt_for_hex, "%s%lu%s", "%0", init_hex_fmt, "lx");
  }
  j = 0;
  for(i = a->size - 1; i > -1; i--) {
    if(i == a->size - 1) {
      j += sprintf(buffer, "%lx", a->parts[i]);
    } else {
      j += sprintf(buffer + j, fmt_for_hex, a->parts[i]);
    }
  }
}

void printDecimalToString(char* buffer, bigint* a) {
  bigint t;
  mytype test_num, largest_num_for_base;
  int i, j, digits;
  char fmt_for_base[50];
  largest_num_for_base = 1;
  test_num = 10;
  digits = 0;
  while(test_num >= largest_num_for_base) {
    largest_num_for_base = test_num;
    test_num *= 10;
    digits++;
  }
  convertToBase(&t, a, largest_num_for_base);
  sprintf(fmt_for_base, "%s%d%s", "%0", digits, "lu");
  j = 0;
  for(i = t.size - 1; i > -1; i--) {
    if(i == t.size - 1) {
      j += sprintf(buffer, "%lu", t.parts[i]);
    } else {
      j += sprintf(buffer + j, fmt_for_base, t.parts[i]);
    }
  }
  free(t.parts);
}

void printBaseToString(char* buffer, bigint* a, int b, char *encoding) {
  bigint t;
  int i, j, k, digits, offset;
  mytype largest_num_for_base, test_num, duplicate;
  char fmt_for_base[50];
  largest_num_for_base = 1;
  test_num = b;
  digits = 0;
  while(test_num >= largest_num_for_base) {
    largest_num_for_base = test_num;
    test_num *= b;
    digits++;
  }
  convertToBase(&t, a, largest_num_for_base);
  j = 0;
  for(i = t.size - 1; i > -1; i--) {
    if(i == t.size - 1) {
      j = 0;
      buffer[j + 1] = '\0';
      duplicate = t.parts[i];
      while (duplicate > 0) {
        j++;
        duplicate /= b;
      }
      duplicate = t.parts[i];
      offset = digits - j;
      buffer[j--] = '\0';
      while (duplicate > 0) {
        buffer[j--] = encoding[duplicate % b];
        duplicate /= b;
      }
    } else {
      j = digits * (t.size - i) - 1 - offset;
      buffer[j + 1] = '\0';
      duplicate = t.parts[i];
      for(k = 0; k < digits; k++) {
        buffer[j - k] = encoding[0];
      }
      while (duplicate > 0) {
        buffer[j--] = encoding[duplicate % b];
        duplicate /= b;
      }
    }
  }
  free(t.parts);
}
