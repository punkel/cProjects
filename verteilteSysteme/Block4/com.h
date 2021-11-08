#ifndef COM_H_
#define COM_H_

/* Includes */
#include "message.h"

/* Structs */
typedef struct {
    uint16_t ID;
    uint32_t IP;
    uint16_t Port;
} server;

typedef struct {
    server this;
    server prev;
    server next;
} locations;

struct internSend_t{
    server inServer;
    sendMsg* sendInMsg;
};
typedef struct internSend_t internSend;

/* Global Vars */
locations *setting;

/* Functions */
int         creatSocket         (int port);

int         reciveMessage       (int acceptNr, msg* newMsg);

int         sendMessage         (int acceptNr, sendMsg* sendMess);
int         sendIntern          (internSend* sendIntern, int closeCon);
int         sendBack            (int socketNr, msg* mess, int fail);

internSend* creatSendInternMsg  (msg* message, server toServer, int toNext);
uint16_t*   creatModKey         (externalMsg* mess);
int         whereIsEle          (msg *message);
int         thisNode            (uint16_t *hash);

void        printLocals         ();

#endif /* COM_H_ */
