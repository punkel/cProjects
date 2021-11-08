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

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

int creatSocket(char* port) {
    int socketNr = socket(AF_INET, SOCK_STREAM, 0);
    if( socketNr < 0 ){
        DP("S: Can't create socket!\n");
        close(socketNr);
        return ESERVER;
    }

    // configurat server
    struct sockaddr_in server;
    memset( &server, 0, sizeof (server));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind server with socket
    if(bind(socketNr,(struct sockaddr*)&server, sizeof( server)) < 0){
        DP("S: %d\n",bind(socketNr,(struct sockaddr*)&server, sizeof( server)));
        DP("S: Can't bind the socket\n");
        close(socketNr);
        return ESERVER;
    }

    // start listen on socket
    if(listen(socketNr, 1)){
        DP("S: Can't listen on the socket\n");
        close(socketNr);
        return ESERVER;
    }

    DP("S: Server online\n");

    return socketNr;
}

int main( int argc, char *argv[]){
    // Check for Arguments
    if( argc != 2 ){
        DP("S: False number of argument!\n");
        return ESERVER;
    }

    // Create socket
    int socketNr = creatSocket(argv[1]);

    if(socketNr < 0){
        DP("S: Can't create Socket\n");
        return ESERVER;
    }

    // send data to client
    struct sockaddr_in client;
    int acceptNr = 0;
    unsigned int clientLen = 0;

    long messageLen __attribute__((unused)) = 0;

    int fail = 0;
    uint8_t* bufFail = (uint8_t*) calloc( HEADER , sizeof(uint8_t));

    if(bufFail == NULL){
        DP("S: basic space reservation fails\n");
        free(bufFail);
        return ENOMEM;
    }

    clearMessage* recvMess = NULL;

#ifdef DEBUG
for(int j = 0; j<5; j++)
#else
for(;;)
#endif
    {
        clientLen = sizeof(client);
        acceptNr = accept(socketNr, (struct sockaddr*)&client, &clientLen);

        if( acceptNr < 0 ){
            DP("S: Can't accept");
            close(acceptNr);
            continue;
        }

        /*
         * content start
         */

        recvMess = initMessage();
        if(!recvMess){
            DP("S: Message cant init\n");
            close(acceptNr);
            continue;
        }

        if(reciveMessage(acceptNr, recvMess) != 0){
            DP("S: Something wrong with recive\n");
            fail++;
        }

#if 0
        DP("Message: Len = %ld value: ", recvMess->valueLen);
        for(int i = 0; i<recvMess->valueLen; i++){
          printf("%d ", (uint8_t)*(recvMess->value+i));
        }
        DP("\n");
#endif

        *(bufFail) = recvMess->mode;
        messageLen = 0;

        sendM sendMess = {.len = 0, .buffer = NULL};

        if(!fail){
            switch (recvMess->mode) {
                case M_GET:
                    DP("S: GET detect\n");
                    if(!getEle(recvMess)){
                        recvMess->mode += (int)M_ACK;
                        sendMess = creatMessage(recvMess, TRUE);
                    } else {
                        fail++;
                        DP("S: Something wrong\n");
                    }
                    break;
                case M_SET:
                    DP("S: SET detect\n");
                    if(!setEle(recvMess)){
                        recvMess->mode += (int)M_ACK;
                        sendMess = creatMessage(recvMess, FALSE);
                    } else {
                        fail++;
                        DP("S: Something wrong\n");
                    }
                    break;
                case M_DEL:
                    DP("S: DELETE detect\n");
                    if(!delEle(recvMess)){
                        recvMess->mode += (int)M_ACK;
                        sendMess = creatMessage(recvMess, FALSE);
                    } else {
                        fail++;
                        DP("S: Something wrong or del empty\n");
                    }
                    break;
                default:
                    fail++;
                    DP("S: False Mode\n");
                    break;
            }
        }

        DP("S: send ");
        if(sendMess.len > 0){
            messageLen = (int)send( acceptNr, sendMess.buffer, sendMess.len, 0);
            DP("%ld\n", messageLen);
        } else {
            fail++;
            DP("not\n");
        }

        if(fail){
            DP("S: Fail detect, send Header back ");
            for(int i=0; i<HEADER;i++){
                DP("%d", *(bufFail+i));
            }
            DP("\n");
            send( acceptNr, bufFail, HEADER, 0);
        }

        /*
         * content end
         */

        // close connection
        close(acceptNr);
        fail = 0;
        memset(bufFail, 0, HEADER);
        if(sendMess.buffer != NULL) free(sendMess.buffer);
        free(recvMess);
    }
    freeTabel();
    free(bufFail);
    return 0;
}
