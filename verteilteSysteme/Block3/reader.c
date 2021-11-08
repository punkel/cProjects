#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "llist.h"
#include "reader.h"

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

void qPrintSec(void* q){
  quoat* newQ = q;
  if( newQ == NULL)
    return;
  DP("len: %d, Quoat: ", newQ->len);
  for(int i=0;i<newQ->len;i++){
      DP("%c",*(newQ->quoat+i));
  }
  DP("\n");
}

void qPrint(){
  llist_print(my_list, qPrintSec);
}

void qFreeSec(void* q){
  quoat* newQ = q;
  if( newQ == NULL)
    return;
  free(newQ->quoat);
  free(newQ);
}

void qFree(){
    llist_free(my_list, qFreeSec);
}

int addToList(uint8_t* text, int len){
    quoat *newQ = calloc(1, sizeof(quoat));
    newQ->quoat = text;
    newQ->len = len;
    if(llist_push(my_list, (void *)newQ) != 0){
        DP("LL: Something wrong with linked list push\n");
        return 1;
    };
    return 0;
}

quoat* qGet(int nr){
  return (quoat *)llist_pull(my_list, nr);
}

/**
  * Read and check all from File
  * return number of lines
  */

quoat* reader(){
  int lines = 0;
  int saveLen = 0;
  int tmpLen = 100;

    my_list = llist_create(NULL);

    if(my_list != NULL){

        uint8_t *tmpBUF = (uint8_t *)calloc(tmpLen, sizeof(uint8_t));
        if(tmpBUF != NULL) {
            int iter = 0; // iterration var
            int c[1];
            c[0] = 0;
            int check = 1;
            while(check){
                check = read(0, c, (size_t)1);
                tmpBUF[iter] = c[0];
                iter++;
                if(check < 1 || iter > tmpLen-1){
                    iter -= (check < 1 ? 1 : 0); // EOF don't save
                    lines++;
                    saveLen += iter;
                    if(addToList(tmpBUF, iter) != 0){
                        DP("LL: addition Fail\n");
                        qFree();
                        lines = 0;
                        break;
                    }
                    iter = 0;
                    DP("add ok\n");
                    if(check > 0){
                        tmpBUF = (uint8_t *)calloc(tmpLen, sizeof(uint8_t));
                    }
                }
            }
            DP("LL: saveLen = %u, last char = %d\n", saveLen, tmpBUF[iter-1]);
        } else {
            DP("LL: can't alloc mem %d\n", ENOMEM);
            return NULL;
        }
    } else {
        DP("LL: cant initial Linked list\n");
        return NULL;
    }

    quoat *tmp = NULL;
    quoat *ret = malloc(sizeof(quoat));
    if(ret == NULL){
        DP("LL: can't alloc mem %d\n", ENOMEM);
        qFree();
        return NULL;
    }
    ret->len = saveLen;
    ret->quoat = (uint8_t *)calloc(saveLen,sizeof(uint8_t));
    if(ret->quoat == NULL){
        DP("LL: can't alloc mem %d\n", ENOMEM);
        free(ret);
        qFree();
        return NULL;
    }

    for(int i=0; i<lines; i++){
        tmp = qGet(i);
        for(int j=0;j<tmp->len;j++){
            *(ret->quoat+tmpLen*i+j) = *(tmp->quoat+j);
        }
    }
    qFree();

    return ret;
}
