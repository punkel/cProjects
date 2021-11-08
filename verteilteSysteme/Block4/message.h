#ifndef _MESSAGE_H
#define _MESSAGE_H

/* Includes */
#include <stdint.h>

/* Defines */
#define EXTERNHEADER 7
#define INTERNALHEADER 11

// internal Msg
#define CONTROL 128
#define REPLY 2
#define LOOKUP 1

// external Msg
#define M_ACK 8//16
#define M_GET 4//32
#define M_SET 2//64
#define M_DEL 1//128

/* Structs */
struct internalMsg_t {
    uint8_t mode;
    uint16_t hashID;
    uint16_t nodeID;
    uint32_t nodeIP;
    uint16_t nodePort;
};

struct externalMsg_t {
    uint8_t mode;
    uint16_t keyLen;
    uint32_t valueLen;
    unsigned char* key;
    unsigned char* value;
};

struct msg_t {
    struct internalMsg_t* internMsg;
    struct externalMsg_t* externMsg;
};

typedef struct internalMsg_t internalMsg;
typedef struct externalMsg_t externalMsg;
typedef struct msg_t msg;

typedef struct {
  uint8_t *buffer;
  long int len;
} sendMsg;

/* Include for functions needed */
/* Do not change position */
#include "com.h"

/* Functions */
msg*                    initMsg             (int msgType);
sendMsg*                initSendMsg         (long int bufferLen);
internalMsg*            initInternalMsg     ();
externalMsg*            initExternalMsg     ();
internSend*             initInternSend      ();

sendMsg*                creatExternalMsg    (externalMsg* message, int sendValue);
sendMsg*                creatInternalMsg    (internalMsg* message);

sendMsg*                handelExternalMsg   (externalMsg* message, int fail);
struct internSend_t*    handelInternalMsg   (internalMsg* message, int fail);

void                    printMsg            (msg* workMessage);
int                     checkMsg            (msg* mess);

void                    freeSendMsg         (sendMsg *del);
void                    freeMsg             (msg* mess);
void                    freeInternSend      (internSend *del);

#endif /* _MESSAGE_H */
