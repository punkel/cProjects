#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "uthash.h"
#include "llist.h"

#include "storageHandler.h"
#include "com.h"
#include "llist.h"
#include "message.h"
#include "debug.h"


int initAllStore(){
    DP("SH: initStore\n");
    setting = calloc(1,sizeof(locations));
    if(setting == NULL){
        DP("SH: no mem for settings %d\n", ENOMEM);
        return FALSE;
    }

    my_list = llist_create();
    if(my_list == NULL){
        DP("SH: no mem for llist %d\n", ENOMEM);
        free(setting);
        return FALSE;
    }

    return TRUE;
}

void freeAllStore(){
    free(setting);
    freeTabel();
    cFree();
}
/****************************\

        Hash Table

\****************************/

storItem* head;

// init Item
storItem* initEle(){
    DP("SH: initEle\n");
    storItem* ret = calloc(1,sizeof(storItem));
    ret->keyLen = 0;
    ret->valueLen = 0;
    ret->key = NULL;
    ret->value = NULL;
    ret->modKey = NULL;
    return ret;
}

// free full hash table
void freeTabel(){
    HASH_CLEAR(hh,head);
}

// delete reserved storage
void freeEle(storItem* item){
    DP("SH: freeEle\n");
    free(item->key);
    free(item->value);
    free(item->modKey);
    free(item);
}

/*
 * Duration
 * 1 - Message to Item
 * 2 - Item to Message
 */
int copyEle(externalMsg* mess, storItem* item, int durration){

    DP("SH: copyEle\n");
    struct test{
        uint16_t keyLen;
        uint32_t valueLen;
        uint8_t* key;
        uint8_t* value;
    };

    struct test* a = calloc(1,sizeof(struct test));
    struct test* b = calloc(1,sizeof(struct test));
    a->keyLen = 0; a->valueLen = 0; a->key = NULL; a->value = NULL;
    b->keyLen = 0; b->valueLen = 0; b->key = NULL; b->value = NULL;

    if(durration == 1){
        b->keyLen = mess->keyLen;b->valueLen = mess->valueLen;b->key = mess->key;b->value = mess->value;
    } else {
        a->keyLen = mess->keyLen;a->valueLen = mess->valueLen;a->key = mess->key;a->value = mess->value;
        b->keyLen = item->keyLen;b->valueLen = item->valueLen;b->key = item->key;b->value = item->value;
    }

//    DP("\n");
//    DP("mess keyLen %u valueLen %u key %p value %p\n",mess->keyLen,mess->valueLen,mess->key,mess->value);
//    DP("item keyLen %u valueLen %u key %p value %p\n",item->keyLen,item->valueLen,item->key,item->value);
//    DP("\n");
//    DP("a keyLen %u valueLen %u key %p value %p\n",a->keyLen,a->valueLen,a->key,a->value);
//    DP("b keyLen %u valueLen %u key %p value %p\n",b->keyLen,b->valueLen,b->key,b->value);
//    DP("\n");

    a->keyLen   = b->keyLen;
    a->valueLen = b->valueLen;
    a->key      = b->key;
    a->value    = b->value;

    if(durration == 1){
        item->keyLen = a->keyLen;item->valueLen = a->valueLen;item->key = a->key;item->value = a->value;
    } else {
        mess->keyLen = a->keyLen;mess->valueLen = a->valueLen;mess->key = a->key;mess->value = a->value;
    }

    free(a);
    free(b);

    return 0;
}

//print an element
void printEle(storItem* message){
    DP("SH: printEle\n");
    if(message){
        DP("Key length = %u\n", message->keyLen );
        DP("Value length = %u\n", message->valueLen );
        if(message->keyLen){
          DP("Key = ");
          for(uint16_t i=0;i<message->keyLen;i++){
              DP("%c",message->key[i]);
          }
          DP("\n");
        }
        if(message->valueLen){
          DP("Value = [skip]");
          for(uint32_t j=0;j<message->valueLen;j++){
              DP("%c",message->value[j]);
          }
          DP("\n");
        }
    }
}

// add a new element
int setEle(externalMsg *message){
    DP("SH: setEle\n");

    int ret = FALSE;

    if(message){
        storItem* tmp = NULL;

        uint16_t *key = creatModKey(message);
        HASH_FIND(hh,head,key,MODKEYLEN,tmp);

        if(tmp){
            // existing element delete
            HASH_DEL(head, tmp);
            freeEle(tmp);
        }
        DP("SH: key %p %d\n",key,(key ? *(key) : -1));
        // new element
        tmp = initEle();
        tmp->modKey = key;
        if(!tmp){
            DP("SH: tmp %d\n",ENOMEM);
            free(key);
            return ret;
        }

        if(copyEle(message, tmp, 1)){
            DP("somthing wrong\n");
            free(key);
            return ret;
        }
        HASH_ADD_KEYPTR(hh,head,tmp->modKey,MODKEYLEN,tmp);
        ret = TRUE;
    } else {
        DP("fail\n");
    }

    return ret;
}

// return an element
int getEle(externalMsg *message){
    DP("SH: getEle\n");

    int ret = FALSE;

    if(message){
        storItem* tmp = NULL;
        uint16_t *key = creatModKey(message);

        HASH_FIND(hh,head,key,MODKEYLEN,tmp);
        DP("SH: Find par Key = %u keyLen = %u pos = %p\n", *key, message->keyLen, tmp);

        // calloc key from message is useless
        free(message->key);
        if(tmp){
            if(copyEle(message, tmp, 2)){
                DP("SH: somthing wrong\n");
                free(key);
                return 1;
            }
            ret = TRUE;
        } else {
            DP("SH: fail 2\n");
        }

        free(key);

    } else {
        DP("SH: fail 1\n");
    }

    return ret;
}

// delete an element
int delEle(externalMsg *message){
    DP("SH: delEle\n");

    int ret = FALSE;

    if(message){
        storItem* tmp = NULL;
        uint16_t *key = creatModKey(message);

        HASH_FIND(hh,head,key,MODKEYLEN,tmp);

        if(tmp){
            // calloc key from message is useless
            free(message->key);
            HASH_DEL(head, tmp);
            freeEle(tmp);
            ret = TRUE;
        } else {
            DP("SH: fail 2\n");
        }

        free(key);

    } else {
        DP("SH: fail 1\n");
    }

    return ret;
}

int isEleInStore(uint16_t *key){
    DP("SH: isEleInStore %u\n", *key);
    int ret = FALSE;

    if(key == NULL){
        return ret;
    }

    storItem* tmp = NULL;
    HASH_FIND(hh,head,key,MODKEYLEN,tmp);
    if(tmp){
        ret = TRUE;
    }
    DP("SH: Element %s Store\n", (ret? "is in" : "is not in"));
    return ret;
}

/****************************\

        Linklist

\****************************/

saveCon* initSaveCon(){
    saveCon* ret = calloc(1, sizeof(saveCon));
    if(ret != NULL){
        ret->socketNr = 0;
        ret->message = NULL;
        ret->modKey = NULL;
    }
    return ret;
}

void cPrintSec(void* sc){
    saveCon* newSC = sc;
    if( newSC == NULL) return;
    DP("SH: SocketNr: %d\n", newSC->socketNr);
    printMsg(newSC->message);
}

void cPrint(){
    struct node *head = *my_list;
    DP("SH: Print LList next = %p\n",head->next);
    llist_print(my_list, cPrintSec);
}

void cFreeSec(void* sc){
  saveCon* newSC = sc;
  if( newSC == NULL)
    return;
}

void cFree(){
    DP("SH: Free LList\n");
    llist_free(my_list, cFreeSec);
}

int cPush(int socketNr, msg* mess){
    DP("SH: cPush\n");

    if(socketNr < 0 ){
        DP("SH: inc data are NULL\n");
        return FALSE;
    }
    if(!checkMsg(mess)){
        return FALSE;
    }

    saveCon *newSC = initSaveCon();
    if(newSC == NULL){
        DP("SH: init fail\n");
        return FALSE;
    }
    newSC->socketNr = socketNr;
    newSC->message = mess;

    //                                 extern msg                :  intern msg
    if(mess->externMsg != NULL){
        newSC->modKey = creatModKey(mess->externMsg);
        if(newSC->modKey == NULL){
            free(newSC);
            return FALSE;
        }
    } else {
        newSC->modKey = calloc(1, sizeof(uint16_t));
        if(newSC->modKey == NULL){
            DP("SH: Cant alloc key mem %d\n", ENOMEM);
            free(newSC);
            return FALSE;
        }
        memcpy(newSC->modKey, &mess->internMsg->hashID, MODKEYLEN);
    }

    if(llist_push(my_list, (void *)newSC, newSC->modKey, MODKEYLEN) != 0){
        DP("SH: Something wrong with linked list push\n");
        free(newSC);
        return FALSE;
    };

    return TRUE;
}

void cDelEle(uint16_t *modKey){
    DP("SH: cDelEle\n");
    llist_freeNode(my_list, cFreeSec, modKey, MODKEYLEN);
}

saveCon* cGet(uint16_t *modKey){
    DP("SH: cGet\n");
    return (saveCon *)llist_pull(my_list, modKey, MODKEYLEN);
}
