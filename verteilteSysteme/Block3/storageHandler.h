#ifndef _STORAGEHANDLER_H
#define _STORAGEHANDLER_H

#include "uthash.h"
#include "message.h"

typedef struct {
    uint16_t keyLen;
    uint32_t valueLen;
    uint8_t *key;
    uint8_t *value;
    UT_hash_handle hh;
} storItem;

int setEle(clearMessage* message);
int getEle(clearMessage* message);
int delEle(clearMessage* message);
void printStorage();
void freeTabel();

#endif /* _STORAGEHANDLER_H */
