#ifndef _READER_H
#define _READER_H

#include "llist.h"
#include <stdint.h>

typedef struct {
  uint8_t *quoat;
  int len;
} quoat;

llist *my_list;
quoat* reader();
void qFreeSec(void* q);

#endif /* _READER_H */
