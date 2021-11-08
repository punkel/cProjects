#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdint.h>

#define HEADER 7
#define M_ACK 8//16
#define M_GET 4//32
#define M_SET 2//64
#define M_DEL 1//128

typedef struct {
    uint8_t mode;
    uint16_t keyLen;
    uint32_t valueLen;
    unsigned char* key;
    unsigned char* value;
} clearMessage;

typedef struct {
  uint8_t *buffer;
  long int len;
} sendM;

void printClearMessage(clearMessage* workMessage);
clearMessage* initMessage();
int reciveMessage(int acceptNr, clearMessage* newMessage);
sendM creatMessage(clearMessage* message, int sendValue);
void freeMessage(clearMessage* mess);

#endif /* _MESSAGE_H */
