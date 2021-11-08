#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "storageHandler.h"
#include "com.h"

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

#define CLEAR(a,b)  if(a) close(a); \
                    freeMsg(b); \
                    fail = FALSE; \
                    continue;

extern locations *setting;

int main( int argc, char *argv[]){
    // Check for Arguments
    if( argc != 10 ){
        DP("S: False number of argument!\n");
        return ESERVER;
    }

    if(!initAllStore()){
        return ENOMEM;
    }

    // copy input
    setting->this.ID    = atoi(argv[1]);
    setting->this.IP    = ntohl(inet_addr(argv[2]));
    setting->this.Port  = atoi(argv[3]);

    setting->prev.ID    = atoi(argv[4]);
    setting->prev.IP    = ntohl(inet_addr(argv[5]));
    setting->prev.Port  = atoi(argv[6]);

    setting->next.ID    = atoi(argv[7]);
    setting->next.IP    = ntohl(inet_addr(argv[8]));
    setting->next.Port  = atoi(argv[9]);

    if(!setting->this.ID || !setting->prev.ID || !setting->next.ID){
        DP("S: any ID = 0 Don't do that\n");
        freeAllStore();
        return ESERVER;
    }

    // Create socket
    int socketNr = creatSocket(setting->this.Port);

    if(socketNr < 0){
        DP("S: Can't create Socket\n");
        freeAllStore();
        return ESERVER;
    }

    // send data to client
    struct sockaddr_in client;
    int acceptNr = 0;
    unsigned int clientLen = 0;
    int fail = FALSE;
    msg* recvMess = NULL;

#ifdef DEBUG
for(int j = 0; j<8; j++)
#else
for(;;)
#endif
    {
        DP("\nS: new message\n");

        clientLen = sizeof(client);
        acceptNr = accept(socketNr, (struct sockaddr*)&client, &clientLen);
        DP("S: acceptNr = %d\n", acceptNr);
        if( acceptNr < 0 ){
            DP("S: Can't accept");
            CLEAR(acceptNr,NULL);
        }

        recvMess = initMsg(0);
        if(!recvMess){
            CLEAR(acceptNr,NULL);
        }

        if(reciveMessage(acceptNr, recvMess) != 0){
            DP("S: Something wrong with recive\n");
            fail = TRUE;
        }

        if(recvMess->externMsg == NULL && recvMess->internMsg == NULL){
            DP("S: something wrong with handeling\n");
            CLEAR(acceptNr, recvMess);
        }

        DP("S: Extern %s and Intern %s\n",(recvMess->externMsg ? "yes" : "no"),(recvMess->internMsg ? "yes" : "no"));

        if(recvMess->externMsg != NULL){ /* Extern Msg */
            switch (whereIsEle(recvMess)) {
                case 0:
                {
                    /* auf die suche gehen */
                    /* socket + context speichern */

                    if(!cPush(acceptNr, recvMess)){
                        sendBack(acceptNr, recvMess, TRUE);
                        CLEAR(acceptNr, recvMess);
                        break;
                    }

                    internSend* sMess = creatSendInternMsg(recvMess, setting->next, FALSE);

                    if(sMess == NULL){
                        DP("S: some init of Space fail\n");
                        cDelEle(creatModKey(recvMess->externMsg));
                        sendBack(acceptNr, recvMess, TRUE);
                        freeMsg(recvMess);
                        break;
                    }

                    if(sMess->sendInMsg == NULL || sendIntern(sMess, TRUE) < 0){
                        DP("S: internal Msg == NULL or sending fail\n");
                        cDelEle(creatModKey(recvMess->externMsg));
                        sendBack(acceptNr, recvMess, TRUE);
                    }

                    freeInternSend(sMess);
                }
                break;
                case 1:
                    // this node have the information
                    sendBack(acceptNr, recvMess, fail);
                    free(recvMess);
                    CLEAR(acceptNr, NULL);
                    break;
                case 2:
                {
                    // next node have the information
                    internSend* sMess = creatSendInternMsg(recvMess, setting->next, TRUE);

                    if(sMess == NULL){
                        DP("S: some init of Space fail\n");
                        sendBack(acceptNr, recvMess, TRUE);
                        freeMsg(recvMess);
                        break;
                    }

                    msg* backMsg = initMsg(0);

                    int sNr = sendIntern(sMess, FALSE);
                    DP("S: socketNr = %d\n", sNr);
                    if(sNr > 0){
                        if(!reciveMessage(sNr, backMsg)){
                            sendMessage(acceptNr, creatExternalMsg(backMsg->externMsg, TRUE));
                        }// if(!reciveMessage(sNr, backMsg))
                    } else {
                        DP("S:Send intern fail\n");
                        sendBack(acceptNr, recvMess, TRUE);
                    } // if(sNr > 0)

                    cDelEle(&recvMess->internMsg->hashID);
                    freeMsg(backMsg);
                    freeInternSend(sMess);
                    CLEAR(acceptNr, recvMess);
                }
                break;
                default:
                    DP("S: something wrong default case!\n");
                    sendBack(acceptNr, recvMess, TRUE);
                    CLEAR(acceptNr, recvMess);
                    break;
                }
        } else { /* Intern Msg */
            close(acceptNr);
            if( recvMess->internMsg->nodeID     == setting->this.ID &&
                recvMess->internMsg->nodeIP     == setting->this.IP &&
                recvMess->internMsg->nodePort   == setting->this.Port
            ){
                /* ich bin einmal im Kreis gelaufen */
                DP("S: ich bin einmal im Kreis gelaufen\n");
                saveCon* msgFromClient = cGet(&recvMess->internMsg->hashID);

                if(msgFromClient == NULL){
                    DP("S: Dont know from which Client\n");
                    cDelEle(&recvMess->internMsg->hashID);
                    CLEAR(0,recvMess);
                }

                int fail = TRUE;

                sendBack(msgFromClient->socketNr, msgFromClient->message, fail);
                if(msgFromClient->message) free(msgFromClient->message);
                free(msgFromClient);
                cDelEle(&recvMess->internMsg->hashID);
                CLEAR(acceptNr, recvMess);
            }

            internSend* sendMess = handelInternalMsg(recvMess->internMsg, fail);
            if( sendMess == NULL){
                DP("S: sendMess = NULL\n");
                CLEAR(0,recvMess);
            }

            if(recvMess->internMsg->mode == (CONTROL + REPLY)){
                /* senden an node mit externer anfrage */
                DP("S: get Reply\n");
                saveCon* msgFromClient = cGet(&recvMess->internMsg->hashID);
                if(msgFromClient == NULL){
                    DP("S: Dont know from which Client\n");
                    freeInternSend(sendMess);
                    CLEAR(0,recvMess);
                }

                msg* backMsg = initMsg(0);
                sendMess->sendInMsg = creatExternalMsg(msgFromClient->message->externMsg, TRUE);

                if(
                    backMsg == NULL ||
                    sendMess->sendInMsg == NULL
                ) {
                    DP("S:");
                    DP(" sendInMsg = %p", sendMess->sendInMsg);
                    DP(" msgFromClient = %p", msgFromClient);
                    DP("\n");
                }

                int sNr = sendIntern(sendMess, FALSE);
                DP("S: socketNr = %d\n", sNr);
                if(sNr > 0){
                    if(!reciveMessage(sNr, backMsg)){
                        sendMessage(msgFromClient->socketNr, creatExternalMsg(backMsg->externMsg, TRUE));
                    }// if(!reciveMessage(sNr, backMsg))
                } else {
                    DP("S:Send intern fail\n");
                    sendBack(sNr, msgFromClient->message, TRUE);
                } // if(sNr > 0)
                cDelEle(&recvMess->internMsg->hashID);
                free(msgFromClient);
                freeMsg(backMsg);
            } else {
                /* senden zu nächsten node*/
                sendIntern(sendMess, TRUE);
            }//if(recvMess->internMsg->mode == (CONTROL + REPLY))
            freeInternSend(sendMess);
            freeMsg(recvMess);
        } // if(recvMess->externMsg != NULL) else
    }// loop
    freeAllStore();
    return 0;
}
