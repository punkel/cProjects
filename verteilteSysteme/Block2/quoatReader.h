#ifndef _QUOATREADER_H
#define _QUOATREADER_H

#include "llist.h"

typedef struct {
  char *quoat;
  int len;
} quoat;

llist *my_list;
void qPrint(); // print all quoats
void qFree();
quoat* qGet(int nr);
int fileReader(char *fileName);

#endif /* _QUOATREADER_H */
