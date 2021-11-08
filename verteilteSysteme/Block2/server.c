#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

#include "quoatReader.h"

// strg abfang
#include <signal.h>

int creatSocket(char* port) {
    int socketNr = socket(AF_INET, SOCK_STREAM, 0);
    if( socketNr < 0 ){
        DP("S: Can't create socket!\n");
        close(socketNr);
        return 4;
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
        return 5;
    }

    // start listen on socket
    if(listen(socketNr, 1)){
        DP("S: Can't listen on the socket\n");
        close(socketNr);
        return 6;
    }

    DP("S: Server online\n");

    return socketNr;
}

/* main function */
int main( int argc, char *argv[]){
    // Check for Arguments
    if( argc != 3 ){
        DP("S: False number of argument!\n");
        return 1;
    }

    int maxLineNumber = fileReader(argv[2]);

    // Create socket
    int socketNr = creatSocket(argv[1]);

    // send data to client
    struct sockaddr_in client;
    int acceptNr = 0;
    int sendByte __attribute__((unused));
    unsigned int len = 0;
    int randNum = 0;
    char **buffer = malloc(sizeof(char *));
    *buffer = NULL;

    quoat *sendQuoat;


#ifdef DEBUG
    for(int j = 0;j<10;j++)
#else
    for(;;)
#endif
    {
        len = sizeof(client);
        acceptNr = accept(socketNr, (struct sockaddr*)&client, &len);

        if( acceptNr < 0 ){
            DP("S: Can't accept\n");
            close(acceptNr);
            continue;
        }

        DP("S: client connect\n");

        // calc random number
        randNum = rand()%maxLineNumber;
        DP("S: randNum = %d\n",randNum);

        sendQuoat = qGet(randNum);

        buffer[0] = sendQuoat->quoat;
        len = (unsigned int)sendQuoat->len;

        // send data to client
        if(len > 0)
            sendByte = (int)send( acceptNr, buffer[0], len, 0);
        else{
            DP("S: no quoat get\n");
            continue;
        }
        DP("S: send %d bytes\n", sendByte);

        // close connection
        close(acceptNr);
    } // for(;;)
    qFree();
    close(socketNr);
    free(*buffer);
    free(buffer);
    return 0;
}
