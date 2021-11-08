#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>

#include "message.h"
#include "com.h"
#include "storageHandler.h"

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

void printInternalMsg(internalMsg* extM){
    if(extM != NULL){
        DP("M: Print internMsg\n");
        DP("Mode = %u, hashID = %u, nodeID = %u, nodeIP = %u.%u.%u.%u, nodePort = %u\n",
            extM->mode,
            extM->hashID,
            extM->nodeID,
            extM->nodeIP>>24,((extM->nodeIP<<8)>>24),((extM->nodeIP<<16)>>24),((extM->nodeIP<<24)>>24),
            extM->nodePort
        );
    }
}

void printExternalMsg(externalMsg* intM){
    if(intM != NULL){
        DP("M: Print externMsg\n");
        DP("Mode = %u\n", intM->mode );
        DP("Key length = %u\n", intM->keyLen );
        DP("Value length = %u\n", intM->valueLen );
        if(intM->keyLen){
            DP("Key = ");
            for(uint16_t i=0;i<intM->keyLen;i++){
              DP("%c",intM->key[i]);
            }
            DP("\n");
        }
        if(intM->valueLen){
          DP("Value = ");
          for(uint32_t j=0;j<intM->valueLen;j++){
              printf("%c",intM->value[j]);
          }
          DP("\n");
        }
    }
}

void printMsg(msg* workMessage){

    if(!checkMsg(workMessage)){
        return;
    }
    printInternalMsg(workMessage->internMsg);
    printExternalMsg(workMessage->externMsg);
}

void freeSendMsg(sendMsg* del){
    DP("M: freeSendMsg\n");
    if(del->buffer != NULL) free(del->buffer);
    free(del);
}

sendMsg* initSendMsg(long bufferLen){
    DP("M: initSendMsg\n");
    sendMsg* ret = calloc(1 , sizeof(sendMsg));
    if(ret == NULL){
        DP("M: basic space reservation fails %d\n", ENOMEM);
        return NULL;
    }
    if(bufferLen > 0){
        ret->buffer = calloc( bufferLen , sizeof(uint8_t));
        if(ret->buffer == NULL){
            DP("M: basic space reservation fails %d\n", ENOMEM);
            free(ret);
            return NULL;
        }
    } else {
        ret->buffer = NULL;
    }
    ret->len = bufferLen;
    return ret;
}

internalMsg* initInternalMsg(){
    DP("M: initInternalMsg\n");
    internalMsg* ret = calloc(1,sizeof(internalMsg));
    if(ret != NULL){
        ret->mode       = 0;
        ret->hashID     = 0;
        ret->nodeID     = 0;
        ret->nodeIP     = 0;
        ret->nodePort   = 0;
    } else {
        DP("M: ENOMEM %d \n",ENOMEM);
    }
    return ret;
}

externalMsg* initExternalMsg(){
    DP("M: initExternalMsg\n");
    externalMsg* ret = calloc(1,sizeof(externalMsg));
    if(ret != NULL){
        ret->mode       = 0;
        ret->keyLen     = 0;
        ret->valueLen   = 0;
        ret->key        = NULL;
        ret->value      = NULL;
    } else {
        DP("M: ENOMEM %d \n",ENOMEM);
    }
    return ret;
}

/*
 *  initMsg
 *  input msgType
 *  0 - only msg
 *  1 - with externalMsg
 *  2 - with internalMsg
 */
msg* initMsg(int msgType){
    DP("M: initMsg\n");
    msg* ret = calloc(1,sizeof(msg));
    if(ret != NULL){
        switch (msgType) {
            case 0 /* only Msg */:
                ret->externMsg = NULL;
                ret->internMsg = NULL;
                break;
            case 1 /* externalMsg */:
                ret->externMsg = initExternalMsg();
                ret->internMsg = NULL;
                break;
            case 2 /* internalMsg */:
                ret->internMsg = initInternalMsg();
                ret->externMsg = NULL;
                break;
            default /* wrong */:
                DP("M: wrong msgType\n");
                free(ret);
                ret = NULL;
                break;
        }
    } else {
        DP("M: no space %d\n", ENOMEM);
    }
    return ret;
}

void freeMsg(msg* mess){
    DP("M: freeMsg\n");
    if(mess){
        if(mess->internMsg != NULL) free(mess->internMsg);
        if(mess->externMsg != NULL){
            if(mess->externMsg->key) free(mess->externMsg->key);
            if(mess->externMsg->value) free(mess->externMsg->value);
            free(mess->externMsg);
        }
        free(mess);
    }
}

int checkMsg(msg* mess){
    DP("M: checkMsg\n");
    int ret = FALSE;
    if(mess){
        if(mess->externMsg || mess->internMsg){
            ret = TRUE;
        } else {
            DP("M: externMsg and internMsg empty\n");
        }
    } else {
        DP("M: Msg is empty\n");
    }
    return ret;
}

void freeInternSend(internSend *del){
    DP("M: freeInternSend\n");
    if(del->sendInMsg != NULL) freeSendMsg(del->sendInMsg);
    free(del);
}

internSend* initInternSend(){
    DP("M: initInternSend\n");
    internSend* ret = calloc(1,sizeof(internSend));
    if(ret != NULL){
        ret->inServer.ID   = 0;
        ret->inServer.IP   = 0;
        ret->inServer.Port = 0;
        ret->sendInMsg     = NULL;
    } else {
        DP("M: no space %d\n", ENOMEM);
    }
    return ret;
}

sendMsg* creatInternalMsg(internalMsg* message){
    DP("M: creatInternalMsg\n");
    sendMsg *ret = NULL;
    if(message){
        // creat sendbuffer
        ret = initSendMsg((long int)INTERNALHEADER);
        if( ret == NULL){
            return ret;
        }
        // creat Head
        uint16_t tmpHashID = htons(message->hashID);
        uint16_t tmpNodeID = htons(message->nodeID);
        uint32_t tmpNodeIP = htonl(message->nodeIP);
        uint16_t tmpNodePort = htons(message->nodePort);

        memcpy(ret->buffer, (void *)(&message->mode), sizeof(uint8_t));
        memcpy(ret->buffer+1, &tmpHashID, sizeof(uint16_t));
        memcpy(ret->buffer+3, &tmpNodeID, sizeof(uint16_t));
        memcpy(ret->buffer+5, &tmpNodeIP, sizeof(uint32_t));
        memcpy(ret->buffer+9, &tmpNodePort, sizeof(uint16_t));
    } else {
        DP("M: Something wrong with buff or message\n");
    }
    return ret;
}

/*
 * sendValue
 * 0 - false
 * 1 - true
 */
sendMsg* creatExternalMsg(externalMsg* message, int sendValue){
    DP("M: creatExternalMsg\n");
    sendMsg* ret = NULL;
    if(message){
        // creat sendbuffer
        ret = initSendMsg((long int)( EXTERNHEADER + (sendValue ? message->keyLen + message->valueLen : 0)));
        if( ret == NULL){
            return ret;
        }
        if(sendValue){
            // copy key and/or value to sendbuffer
            DP("M: sendValue\n");
            if(message->keyLen > 0){
                memcpy(ret->buffer+EXTERNHEADER, (void *)message->key, (sizeof(uint8_t))*message->keyLen);
            }
            if(message->valueLen > 0){
                memcpy(ret->buffer+EXTERNHEADER+message->keyLen, (void *)message->value, (sizeof(uint8_t))*message->valueLen);
            }
        } else {
            // send only Header
            // clean Key and value length
            message->keyLen = 0;
            message->valueLen = 0;
        }
        // creat Head
        uint16_t tmpKeyLen = htons(message->keyLen);
        uint32_t tmpValueLen = htonl(message->valueLen);
        memcpy(ret->buffer, (void *)(&message->mode), sizeof(uint8_t));
        memcpy(ret->buffer+1, &tmpKeyLen, sizeof(uint16_t));
        memcpy(ret->buffer+3, &tmpValueLen, sizeof(uint32_t));
    } else {
        DP("M: Something wrong with buff or message\n");
    }
    return ret;
};

sendMsg* handelExternalMsg(externalMsg* message, int fail){
    DP("M: handelExternalMsg\n");
    sendMsg* ret = NULL;
    if(fail)DP("M: fail inc\n");

    if(!fail){
        switch (message->mode) {
            case M_GET:
            DP("M: GET detect\n");
            if(getEle(message)){
                message->mode += (int)M_ACK;
                printExternalMsg(message);
                ret = creatExternalMsg(message, TRUE);
            } else {
                fail = TRUE;
            }
            break;
            case M_SET:
            DP("M: SET detect\n");
            if(setEle(message)){
                message->mode += (int)M_ACK;
                ret = creatExternalMsg(message, FALSE);
            } else {
                fail = TRUE;
            }
            break;
            case M_DEL:
            DP("M: DELETE detect\n");
            if(delEle(message)){
                message->mode += (int)M_ACK;
                ret = creatExternalMsg(message, FALSE);
            } else {
                fail = TRUE;
            }
            break;
            default:
            fail = TRUE;
            DP("M: False Mode\n");
            break;
        }
        if(ret == NULL){
            fail = TRUE;
        }
    }

    if(fail){
        DP("M: Fail during handeling\n");
        ret = initSendMsg(EXTERNHEADER);
        if(ret != NULL){
            memcpy(ret->buffer, (void *)&message->mode, sizeof(uint8_t));
            ret->len = EXTERNHEADER;
        } else {
            DP("M: Something failed\n");
        }
    }

    return ret;
}

internSend* handelInternalMsg(internalMsg* message, int fail){
    DP("M: handelInternalMsg\n");
    internSend* ret = NULL;
    if(fail)DP("M: fail inc\n");

    if(!fail){
        switch (message->mode) {
            case (CONTROL + LOOKUP ):
                DP("M: CONTROL + LOOKUP\n");
                ret = initInternSend();
                if(ret != NULL){
                    msg* tmp = initMsg(0);
                    tmp->internMsg = message;
                    int check = whereIsEle(tmp);
                    if(check>0){
                        /* gesuchtes Element ist in diesem Node */
                        internalMsg* tmpIMsg = initInternalMsg();
                        ret->inServer.ID    = message->nodeID;
                        ret->inServer.IP    = message->nodeIP;
                        ret->inServer.Port  = message->nodePort;
                        tmpIMsg->mode       = CONTROL + REPLY;

                        server toServer = {.ID=0, .IP=0, .Port=0};
                        if(check==1){
                            toServer.ID     = setting->this.ID;
                            toServer.IP     = setting->this.IP;
                            toServer.Port   = setting->this.Port;
                        } else {
                            toServer.ID     = setting->next.ID;
                            toServer.IP     = setting->next.IP;
                            toServer.Port   = setting->next.Port;
                        }

                        tmpIMsg->nodeID     = toServer.ID;
                        tmpIMsg->nodeIP     = toServer.IP;
                        tmpIMsg->nodePort   = toServer.Port;

                        tmpIMsg->hashID     = message->hashID;
                        ret->sendInMsg      = creatInternalMsg(tmpIMsg);
                        free(tmpIMsg);
                    } else {
                        /* gesuchtes Element ist nicht in diesem Node */
                        ret->inServer.ID    = setting->next.ID;
                        ret->inServer.IP    = setting->next.IP;
                        ret->inServer.Port  = setting->next.Port;
                        ret->sendInMsg      = creatInternalMsg(message);
                    }
                    free(tmp);
                } else {
                    fail = TRUE;
                }
                break;
            case (CONTROL + REPLY):
                DP("M: CONTROL + REPLY\n");
                ret = initInternSend();
                if(ret != NULL){
                    ret->inServer.ID    = message->nodeID;
                    ret->inServer.IP    = message->nodeIP;
                    ret->inServer.Port  = message->nodePort;
                } else {
                    fail = TRUE;
                }
                break;
            case CONTROL:
            /* no break */
            /* fall through */
            default:
            fail = TRUE;
            DP("M: False Mode\n");
            break;
        }
        if(ret == NULL){
            fail = TRUE;
        }
    }

    if(fail){
        DP("M: Fail during handeling\n");
        ret = NULL;
        if(ret != NULL){
            //memcpy(ret->buffer, (void *)&message->mode, sizeof(uint8_t));
            //ret->len = INTERNALHEADER;
        } else {
            DP("M: Something failed\n");
        }
    }

    return ret;
}
