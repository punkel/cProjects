#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>

#include "message.h"

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

clearMessage* initMessage(){
    DP("M: initMessage\n");
    clearMessage* ret = malloc(sizeof(clearMessage));
    ret->mode = 0;
    ret->keyLen = 0;
    ret->valueLen = 0;
    ret->key = NULL;
    ret->value = NULL;
    return ret;
}

void freeMessage(clearMessage* mess){
    DP("M: freeMessage M %p | K %p | V %p\n",mess,mess->key,mess->value);
    free(mess->key);
    free(mess->value);
    free(mess);
}

int decodeHeader(uint8_t* incMessage, clearMessage* workMessage){
    DP("M: decodeHeader\n");
    workMessage->mode = *incMessage;
    workMessage->keyLen = ntohs((*((uint16_t *)(incMessage+1))));
    workMessage->valueLen = ntohl((*((uint32_t *)(incMessage+3))));
    return 0;
}

int reciveMessage(int acceptNr, clearMessage* newMessage){
    DP("M: reciveMessage\n");

    if(newMessage == NULL){
        DP("M: newMessage %d\n",ENOMEM);
        return ENOMEM;
    }

    uint8_t tmpHeaderBuf[HEADER];
    uint32_t saveLen = 0;
    const uint32_t maxRecvLen = 1024;
    int reciveLen = (int)recv(acceptNr, tmpHeaderBuf, (size_t)(HEADER), 0);

    DP("M: Recive %d for HEADER\n",reciveLen);
    if(reciveLen < 0){
        return EUNKNOW;
    }
    saveLen += reciveLen;

    decodeHeader(tmpHeaderBuf, newMessage);

    if( newMessage->keyLen > 0 ){
        newMessage->key = calloc((size_t)newMessage->keyLen, sizeof(uint8_t));
        if(newMessage->key == NULL){
          DP("M: key %d\n",ENOMEM);
          return ENOMEM;
        }
        uint32_t restLen = newMessage->keyLen;
        do {
            reciveLen = (int)recv(acceptNr, (newMessage->key+(saveLen-HEADER)), (restLen > maxRecvLen ? maxRecvLen : restLen), 0);
            if(reciveLen < 0){
                DP("M: Recive %d for Key\n",reciveLen);
                return EUNKNOW;
            }
            saveLen += reciveLen;
            restLen -= reciveLen;
        }while(saveLen < (uint32_t)(HEADER + newMessage->keyLen));
        DP("M: Recive %d for Key\n",saveLen);
    }

    if( newMessage->valueLen > 0 ){
        newMessage->value = calloc((size_t)newMessage->valueLen, sizeof(uint8_t));
        if(newMessage->value == NULL){
          DP("M: value %d\n",ENOMEM);
          return ENOMEM;
        }
        uint32_t restLen = newMessage->valueLen;
        do {
            reciveLen = (int)recv(acceptNr, (newMessage->value+(saveLen-HEADER-newMessage->keyLen)), (restLen > maxRecvLen ? maxRecvLen : restLen), 0);
            if(reciveLen < 0){
                DP("M: Recive %d for Value\n",reciveLen);
                return EUNKNOW;
            }
            saveLen += reciveLen;
            restLen -= reciveLen;
            DP("recL = %d|",reciveLen);
        }while(saveLen < (uint32_t)(HEADER + newMessage->keyLen + newMessage->valueLen));
        DP("M: Recive %d for Value\n",saveLen);
    }

    if(saveLen != (HEADER + newMessage->keyLen + newMessage->valueLen)){
        DP("M: logic fail Header = %d calcKeyLen = %d calcValueLen = %d reciveLen = %d\n",
            (int)HEADER,
            newMessage->keyLen,
            newMessage->valueLen,
            saveLen
        );
        return EUNKNOW;
    }

    return 0;
}

/*
 * sendValue
 * 0 - false
 * 1 - true
 */

sendM creatMessage(clearMessage* message, int sendValue){
    DP("M: creatMessage\n");
    sendM ret;
    ret.len = 0;
    if(message){
        // creat sendbuffer
        ret.buffer = calloc(( HEADER + (sendValue ? message->keyLen + message->valueLen : 0)),sizeof(uint8_t));
        if( ret.buffer == NULL){
            DP("M: no mem alloc\n");
            ret.len = ENOMEM;
            return ret;
        }
        if(sendValue){
            // copy key and/or value to sendbuffer
            DP("M: sendValue\n");
            if(message->keyLen > 0){
                memcpy(ret.buffer+HEADER, (void *)message->key, (sizeof(uint8_t))*message->keyLen);
            }
            if(message->valueLen > 0){
                memcpy(ret.buffer+HEADER+message->keyLen, (void *)message->value, (sizeof(uint8_t))*message->valueLen);
            }
            ret.len += message->keyLen + message->valueLen;
        } else {
            // send only Header
            // clean Key and value length
            message->keyLen = 0;
            message->valueLen = 0;
        }
        // creat Head
        uint16_t tmpKeyLen = htons(message->keyLen);
        uint32_t tmpValueLen = htonl(message->valueLen);
        memcpy(ret.buffer, (void *)(&message->mode), sizeof(uint8_t));
        memcpy(ret.buffer+1, &tmpKeyLen, sizeof(uint16_t));
        memcpy(ret.buffer+3, &tmpValueLen, sizeof(uint32_t));
        ret.len += ((int)sizeof(uint8_t))*HEADER;
    } else {
        DP("M: Something wrong with buff or message\n");
    }
    return ret;
};

void printClearMessage(clearMessage* workMessage){
    DP("M: printClearMessage\n");
    if(workMessage){
      DP("Mode = %u\n", workMessage->mode );
      DP("Key length = %u\n", workMessage->keyLen );
      DP("Value length = %u\n", workMessage->valueLen );
      if(workMessage->keyLen){
          DP("Key = ");
          for(uint16_t i=0;i<workMessage->keyLen;i++){
              DP("%c",workMessage->key[i]);
          }
          DP("\n");
      }
      if(workMessage->valueLen){
          DP("Value =");
          for(uint32_t j=0;j<workMessage->valueLen;j++){
              printf("%c",workMessage->value[j]);
          }
          DP("\n");
      }
    }
}
