#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "message.h"
#include "reader.h"

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

int main( int argc, char *argv[]){
    // Check for Arguments

    if( argc < 4 || argc > 6 ){
        DP("C: False number of argument!\n");
        return 1;
    }

    /* Get terminal inc */
    //vars
    quoat *fromLine = NULL;
    clearMessage* sendMessage = initMessage();

    // send message
    // mode detect

    if( !strncmp(argv[3], "SET", 3 ) ){
      DP("Set fount\n");
      sendMessage->mode = (int)M_SET;
    } else if( !strncmp(argv[3], "GET", 3 ) ){
      DP("Get found\n");
      sendMessage->mode = (int)M_GET;
    } else if( !strncmp(argv[3], "DELETE", 6 ) ){
      DP("Delete found\n");
      sendMessage->mode = (int)M_DEL;
    } else {
      DP("false mode\n");
      return 10;
    }

    sendMessage->key    = (uint8_t* )argv[4];
    sendMessage->keyLen = strlen(argv[4]);

    if(sendMessage->mode == (int)M_SET){
        if(argc == 5){
            // read from line
            DP("C: read from Line\n");
            fromLine = reader();
            sendMessage->value    = fromLine->quoat;
            sendMessage->valueLen = fromLine->len;
        } else {
            sendMessage->value    = (uint8_t* )argv[5];
            sendMessage->valueLen = strlen(argv[5]);
        }
    }

    /* Get terminal inc */

    // Create socket
    int socketNr = -1;

    socketNr = socket(AF_INET, SOCK_STREAM, 0);
    if( socketNr < 0 ){
        DP("C: Can't create socket!\n");
        return 2;
    }

    // write server infos
    struct in_addr **ipFromAddr;
    struct hostent *host = gethostbyname(argv[1]);

    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[2]));

    if(inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) != 0){
        DP("C: IP addr found\n");
    }else if(host){
        DP("C: DNS found ");

        ipFromAddr = (struct in_addr **)host->h_addr_list;
        if(inet_pton(AF_INET, inet_ntoa( **ipFromAddr), &serveraddr.sin_addr) != 0){
            DP("\n");
        } else {
            DP("but something wrong\n");
            return 4;
        }
    }else{
        DP("no IP or DNS found\n");
        return 5;
    }

    // connect to server
    if( connect(socketNr, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) != 0){
        DP("C: Can't connect to server!\n");
        return 3;
    }

    /************* content ****************/

    sendM sendMess = creatMessage(sendMessage, TRUE);
    if(sendMess.len < 0){
        DP("C: Something wrong! %ld\n", sendMess.len);
        return sendMess.len;
    }

//#ifdef DEBUG
#if 0
    DP("Message: Len = %ld value: ", sendMess.len);
    for(int i = 0; i<sendMess.len; i++){
      printf("%c", (uint8_t)*(sendMess.buffer+i));
    }
    DP("\n");
#endif

    long numByte __attribute__((unused)) = 0; // length of send and recive message
    clearMessage* recvMessage = initMessage();

    // send
    DP("C: Send ");
    if(sendMess.len){
        numByte = (int)send(socketNr, sendMess.buffer, (size_t)sendMess.len, 0);
        DP("%ld bytes\n",numByte);

        numByte = reciveMessage(socketNr, recvMessage);

        if(numByte != 0){
            DP("C: Something wrong! %ld\n", numByte);
            return numByte;
        }

        if(recvMessage->mode == (int)(M_GET + M_ACK)){
            DP("C: print Message\n");
            for(uint32_t i=0; i<recvMessage->valueLen ; i++){
                printf("%c", (char)*(recvMessage->value+i));
            }
        }
        DP("\n");
    } else {
        DP("not\n");
    }

    /************* content ****************/

    // close connection
    if(fromLine != NULL) qFreeSec(fromLine);
    free(sendMess.buffer);
    free(sendMessage); // Don' freeMessage because no space for Key and Value
    freeMessage(recvMessage);
    close(socketNr);
	return 0;
}
