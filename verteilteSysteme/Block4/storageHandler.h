#ifndef _STORAGEHANDLER_H
#define _STORAGEHANDLER_H

/* Includes */
#include <stdint.h>
#include "uthash.h"
#include "llist.h"
#include "message.h"

/* Defines */
#define MODKEYLEN (int)2 // Bytes

/* Structs */
typedef struct {
    uint16_t keyLen;
    uint32_t valueLen;
    uint8_t *key;
    uint8_t *value;
    uint16_t *modKey;
    UT_hash_handle hh;
} storItem;

typedef struct {
    int socketNr;
    msg *message;
    uint16_t *modKey;
} saveCon;

/* Global Vars */
llist *my_list;

/* Functions */
int         initAllStore    ();
void        printStorage    ();

int         setEle          (externalMsg *message);
int         getEle          (externalMsg *message);
int         delEle          (externalMsg *message);
int         isEleInStore    (uint16_t *key);

void        cPrint          ();
int         cPush           (int socketNr, msg* mess);
saveCon*    cGet            (uint16_t *modKey);
void        cDelEle         (uint16_t *modKey);

void        freeAllStore    ();
void        freeTabel       ();
void        cFree           ();


#endif /* _STORAGEHANDLER_H */
