#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

int main( int argc, char *argv[]){
    // Check for Arguments
    if( argc != 3 ){
        DP("C: False number of argument!\n");
        return 1;
    }

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

    // recive data
    int len = 512;
    char buffer[len];
    int numByte = 0;
    memset( buffer, 0, len);


    do{
        numByte = (int)recv(socketNr, buffer, (size_t)len, 0);
        DP("C: recive Bytes: %d\n", numByte);

        // write data to the terminal
        for(int i = 0; i < numByte; i++){
            /*
             * it nothing changs on the meaning of the stream, its only the solution for the windows/linux lineending problem
             */
            switch (buffer[i]) {
                case 13 /*CR*/:
                    if((i+1) < numByte ){
                        if(buffer[i+1] == 10/*LF*/){
                            i++;
                        }
                    }
                    /* no break */
                /* fall through */
//                case 10 /*LF*/:
//                    fwrite("\n", sizeof(char), 1, stdout);
//                    break;
                default:
                    fwrite(buffer+i, sizeof(char), 1, stdout);
                    break;
            }
        }
    }while( numByte );

    // close connection
    close(socketNr);
	return 0;
}
