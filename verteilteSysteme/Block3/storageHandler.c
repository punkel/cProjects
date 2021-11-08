#include <stdio.h>
#include <stdint.h>

#include "uthash.h"

#include "storageHandler.h"
#include "message.h"
#include "debug.h"

storItem* head;

// init Item
storItem* initEle(){
    DP("SH: initEle\n");
    storItem* ret = malloc(sizeof(storItem));
    ret->keyLen = 0;
    ret->valueLen = 0;
    ret->key = NULL;
    ret->value = NULL;
    return ret;
}

// delete storItem
void delStorEle(storItem * del){
    free(del->key);
    free(del->value);
    free(del);
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
    free(item);
}

/*
 * Duration
 * 1 - Message to Item
 * 2 - Item to Message
 */
int copyEle(clearMessage* mess, storItem* item, int durration){

    DP("SH: copyEle\n");
    struct test{
        uint16_t keyLen;
        uint32_t valueLen;
        uint8_t* key;
        uint8_t* value;
    };

    struct test* a = malloc(sizeof(struct test));
    struct test* b = malloc(sizeof(struct test));
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
int setEle(clearMessage* message){
    DP("SH: addEle ");
    int ret = 1;

    if(message){
        storItem* tmp = NULL;

        HASH_FIND(hh,head,message->key,message->keyLen,tmp);

        if(tmp){
            // existing element delete
            HASH_DEL(head, tmp);
            delStorEle(tmp);
        }

        // new element
        tmp = initEle();
        if(!tmp){
            DP("SH: tmp %d\n",ENOMEM);
            return ENOMEM;
        }

        if(copyEle(message, tmp, 1)){
            DP("somthing wrong\n");
            return 1;
        }

        HASH_ADD_KEYPTR(hh,head,tmp->key,tmp->keyLen,tmp);
        DP("succ\n");
        ret = 0;
    } else {
        DP("fail\n");
    }

    return ret;
}

// return an element
int getEle(clearMessage* message){
    DP("SH: getEle ");
    int ret = 1;

    if(message){
        storItem* tmp = NULL;
        HASH_FIND(hh,head,message->key,message->keyLen,tmp);
        DP("SH: Find par Key = %s keyLen = %u\n", message->key,message->keyLen);

        // calloc key from message is useless
        free(message->key);
        if(tmp){
            if(copyEle(message, tmp, 2)){
                DP("SH: somthing wrong\n");
                return 1;
            }
            DP("succ\n");
            ret = 0;
        } else {
            DP("fail 2\n");
        }
    } else {
        DP("fail 1\n");
    }

    return ret;
}

// delete an element
int delEle(clearMessage* message){
    DP("SH: delEle ");
    int ret = 1;

    if(message){
        storItem* tmp = NULL;
        HASH_FIND(hh,head,message->key,message->keyLen,tmp);

        if(tmp){
            DP("succ\n");
            // calloc key from message is useless
            free(message->key);
            HASH_DEL(head, tmp);
            delStorEle(tmp);
            ret = 0;
        } else {
            DP("fail 2\n");
        }
    } else {
        DP("fail 1\n");
    }

    return ret;
}
