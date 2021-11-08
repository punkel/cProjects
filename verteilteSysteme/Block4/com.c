#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "com.h"
#include "storageHandler.h"

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

void printServer(server prt){
    DP("ID: = %u, IP = %u.%u.%u.%u, Port = %u\n",
        prt.ID,
        prt.IP>>24,((prt.IP<<8)>>24),((prt.IP<<16)>>24),((prt.IP<<24)>>24),
        prt.Port
    );
}

void printLocals(){
    DP("C: printLocals\n");
    DP("This: "); printServer(setting->this);
    DP("Prev: "); printServer(setting->prev);
    DP("Next: "); printServer(setting->next);
};

// creat listen port
int creatSocket(int port){
    DP("C: creatSocket\n");
    int socketNr = socket(AF_INET, SOCK_STREAM, 0);
    if( socketNr < 0 ){
        DP("C: Can't create socket!\n");
        close(socketNr);
        return ESERVER;
    }

    // configurat server
    struct sockaddr_in server;
    memset( &server, 0, sizeof (server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind server with socket
    if(bind(socketNr,(struct sockaddr*)&server, sizeof( server)) < 0){
        DP("C: %d\n",bind(socketNr,(struct sockaddr*)&server, sizeof( server)));
        DP("C: Can't bind the socket\n");
        close(socketNr);
        return ESERVER;
    }

    // start listen on socket
    if(listen(socketNr, 1)){
        DP("C: Can't listen on the socket\n");
        close(socketNr);
        return ESERVER;
    }

    DP("C: Server online\n");

    return socketNr;
}

int decodeHeader(int socketNr, msg* workMessage){
    DP("C: decodeHeader\n");

    if(workMessage == NULL){
        return ENOMEM;
    }

    uint8_t tmpMode[1];

    int reciveLen = (int)recv(socketNr, tmpMode, sizeof(uint8_t), 0);

    if(reciveLen < 0){
        return EUNKNOW;
    }

    uint8_t msgLen = ((tmpMode[0] & CONTROL)? 10 : 6);
    uint8_t *incMessage = calloc(msgLen,sizeof(uint8_t));

    reciveLen += (int)recv(socketNr, incMessage, (size_t)(msgLen), 0);
    DP("C: Recive %d for HEADER\n",reciveLen);
    if(reciveLen < 0){
        return EUNKNOW;
    }

    if(tmpMode[0] & CONTROL){
        DP("C: get internal Header\n");
        internalMsg* tmpIntern = initInternalMsg();
        if(tmpIntern == NULL){DP("C: fail\n"); return ENOMEM;}
        tmpIntern->mode         = *tmpMode;
        tmpIntern->hashID       = ntohs(*((uint16_t *)(incMessage)));
        tmpIntern->nodeID       = ntohs((*((uint16_t *)(incMessage+2))));
        tmpIntern->nodeIP       = ntohl((*((uint32_t *)(incMessage+4))));
        tmpIntern->nodePort     = ntohs((*((uint16_t *)(incMessage+8))));
        workMessage->internMsg  = tmpIntern;
    } else {
        DP("C: get external Header\n");
        externalMsg* tmpExtern  = initExternalMsg();
        if(tmpExtern == NULL){DP("C: fail\n"); return ENOMEM;}
        tmpExtern->mode         = *tmpMode;
        tmpExtern->keyLen       = ntohs((*((uint16_t *)(incMessage))));
        tmpExtern->valueLen     = ntohl((*((uint32_t *)(incMessage+2))));
        workMessage->externMsg  = tmpExtern;
    }

    free(incMessage);
//    DP("C: Header = ");
//    DP("Mode = %d, KeyLen = %d, valueLen = %d", workMessage->mode, workMessage->keyLen, workMessage->valueLen);
//    DP("\n");
    return 0;
}

// input: uint8_t* buf == NULL, uint32_t bufLen, int socketNr
// return: sendMsg struct
sendMsg* rec(uint32_t bufLen, int socketNr){
    DP("C: rec\n");
    sendMsg *ret = initSendMsg((long int)bufLen);

    if(ret == NULL){
        return ret;
    }

    ret->len = 0;
    const uint32_t maxRecvLen = 1024;
    int reciveLen = 0;
    uint32_t restLen = bufLen;
    do {
        reciveLen = (int)recv( socketNr, ret->buffer+ret->len, (restLen > maxRecvLen ? maxRecvLen : restLen), 0);
        if(reciveLen < 0){
            DP("C: Recive %d\n",reciveLen);
            free(ret->buffer);
            ret->len = EUNKNOW;
            return ret;
        }
        ret->len += reciveLen;
        restLen -= reciveLen;
    }while(ret->len < bufLen);
    DP("C: recv %ld %p\n", ret->len, ret->buffer);
    return ret;
}


/*
 * Recive Fkt
 *
 * return 0 if ok else less 0
 */
int reciveMessage(int acceptNr, msg* newMsg){
    DP("C: reciveMessage\n");

    if(newMsg == NULL){
        DP("C: newMsg %d\n",ENOMEM);
        return ENOMEM;
    }

    if(decodeHeader(acceptNr, newMsg) != 0){
        DP("C: Header decode fail\n");
        return EUNKNOW;
    }

    if(newMsg->externMsg != NULL){
        externalMsg *newMessage = newMsg->externMsg;
        uint32_t saveLen = (uint32_t)EXTERNHEADER;
        if( newMessage->keyLen > 0 ){
            sendMsg *tmp = rec(newMessage->keyLen, acceptNr);
            if(tmp->len < 0){
                DP("C: Something wrong with rec\n");
                return tmp->len;
            }
            saveLen += tmp->len;
            newMessage->key = tmp->buffer;
            DP("C: Recive %ld for Key\n", tmp->len);
            free(tmp);
        }

        if( newMessage->valueLen > 0 ){
            sendMsg *tmp = rec(newMessage->valueLen, acceptNr);
            if(tmp->len < 0){
                DP("C: Something wrong with rec\n");
                return tmp->len;
            }
            saveLen += tmp->len;
            newMessage->value = tmp->buffer;
            DP("C: Recive %ld for Value\n", tmp->len);
            free(tmp);
        }

        if(saveLen != (EXTERNHEADER + newMessage->keyLen + newMessage->valueLen)){
            DP("C: logic fail Header = %d calcKeyLen = %d calcValueLen = %d reciveLen = %d\n",
            (int)EXTERNHEADER,
            newMessage->keyLen,
            newMessage->valueLen,
            saveLen);
            return EUNKNOW;
        }
    }
    return 0;
}

int sendMessage(int acceptNr, sendMsg* sendMess){
    DP("C: sendMessage\n");
    int ret = FALSE;
    int sendBytes = (int)send( acceptNr, sendMess->buffer, sendMess->len, 0);
    if(sendBytes > 0){
        ret = TRUE;
    }
    DP("C: Send %d Bytes\n", sendBytes);
    return ret;
}

int sendBack(int socketNr, msg* mess, int fail){
    DP("C: sendBack\n");
    int ret = FALSE;
    sendMsg* sendMess = handelExternalMsg(mess->externMsg, fail);
    if(sendMess == NULL){
        DP("C: something wrong with handeling\n");
        close(socketNr);
        freeMsg(mess);
        return ret;
    }

    if(sendMess && sendMess->len > 0){
        ret = sendMessage(socketNr, sendMess);
    } else {
        DP("C: send fail\n");
    }
    freeSendMsg(sendMess);
    close(socketNr);
    return ret;
}

internSend* creatSendInternMsg(msg* message, server toServer, int toNext){
    DP("C: creatSendInternMsg\n");
    internSend* ret = initInternSend();

    ret->sendInMsg = NULL;
    if(toNext){
        if(message && message->externMsg){
            ret->sendInMsg = creatExternalMsg(message->externMsg, TRUE);
        }
    } else {
        internalMsg* iMess      = initInternalMsg();
        uint16_t* tmpKey        = creatModKey(message->externMsg);
        if(iMess == NULL || tmpKey == NULL){
            DP("S: some init of Space fail\n");
            if(!iMess)free(iMess);
            if(!tmpKey)free(tmpKey);
            return NULL;
        }
        iMess->mode             = (uint8_t)(CONTROL + LOOKUP);
        iMess->nodeID           = setting->this.ID;
        iMess->nodeIP           = setting->this.IP;
        iMess->nodePort         = setting->this.Port;

        memcpy(&iMess->hashID, tmpKey, (size_t)MODKEYLEN);

        ret->sendInMsg = creatInternalMsg(iMess);

        free(iMess);
        free(tmpKey);
    }

    ret->inServer.ID    = toServer.ID;
    ret->inServer.IP    = toServer.IP;
    ret->inServer.Port  = toServer.Port;

    return ret;
}

// return
//   -1 - on fail
//    0 - send ok
// else - socketNr
int sendIntern(internSend* sendIntern, int closeCon){
    DP("C: sendIntern\n");
    // Create socket
    int socketNr = -1;
    int ret = -1;

    socketNr = socket(AF_INET, SOCK_STREAM, 0);
    if( socketNr < 0 ){
        DP("C: Can't create socket!\n");
        return ret;
    }

    // write server infos
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(sendIntern->inServer.Port);
    serveraddr.sin_addr.s_addr = htonl(sendIntern->inServer.IP);

    // connect to server
    if( connect(socketNr, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) != 0){
        DP("C: Can't connect to server!\n");
        close(socketNr);
        return ret;
    }

    // send
    DP("C: Send Msg\n");
    if(!sendMessage(socketNr, sendIntern->sendInMsg)){
        DP("C: Something wrong on send!\n");
    } else {
        ret = 0;
    }

    // close connection
    if(closeCon){
        close(socketNr);
    } else {
        ret = socketNr;
    }

    return ret;
}

uint16_t* creatModKey(externalMsg* mess){
    DP("C: creatModKey\n");

    if(mess == NULL){
        DP("C: mess = NULL\n");
        return NULL;
    }

    uint16_t *ret = calloc(1, sizeof(uint16_t));
    if(ret == NULL){
        DP("C: cant calloc modKey %d\n",ENOMEM);
        return ret;
    }

    if(mess->keyLen == 0){
        *ret = 0;
    } else if(mess->keyLen == 1){
        *ret = *mess->key;
    } else {
        *ret = *((uint8_t *)mess->key)<<8 | *((uint8_t *)mess->key+1);
    }

    DP("C: calc key = %u\n", *ret);
    return ret;
}

int betweenNode(uint16_t predID, uint16_t succID, uint16_t hash){
    int ret = FALSE;
    if(hash > predID && hash <= succID){
        ret = TRUE;
    } else if(predID > succID){
        if(hash > predID || hash <= succID){
            ret = TRUE;
        }
    }
    return ret;
}

int nextNode(uint16_t *hash){
    DP("C: nextNode\n");
    return betweenNode(setting->this.ID,setting->next.ID,*hash);
}

int thisNode(uint16_t *hash){
    DP("C: thisNode\n");
    return betweenNode(setting->prev.ID,setting->this.ID,*hash);
}

/*
 * Where is the Element?
 * In: msg
 * out: int
 * FALSE/0 - node don't know it
 * 1 - this node
 * 2 - next node
*/

int whereIsEle (msg *message){
    DP("C: whereIsEle\n");
    int ret = FALSE;

    if(!checkMsg(message)){
        DP("C: inc msg = NULL\n");
        return ret;
    }

    uint16_t *tmpKey = NULL;

    if(message->internMsg){
        DP("C: internMsg search\n");
        tmpKey = &message->internMsg->hashID;
    } else {
        DP("C: externMsg search\n");
        tmpKey = creatModKey(message->externMsg);
    }

    if(thisNode(tmpKey)){
        ret = 1;
    } else if(nextNode(tmpKey)){
        ret = 2;
    } else {
        ret = 0;
    }

    if(message->externMsg != NULL){
        free(tmpKey);
    }
    DP("C: Ele is %s\n",(ret ? (ret == 1 ? "in this Node" : "in next Node") : "unknown"));
    return ret;
}
