#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llist.h"
#include "quoatReader.h"

/* DEBUG Prints */
#include "debug.h"
/* DEBUG Prints */

const int dummLen = 119;
const char dummText[119] = "Ich habe gelernt zu lernen. | Me (default Quoat)\n\nFehler: Auf dem Server konnte die Zitatdatei nicht ausgelesen werden.";
quoat quoateDummy = {   .quoat ="Ich habe gelernt zu lernen. | Me (default Quoat)\n\nFehler: Auf dem Server konnte die Zitatdatei nicht ausgelesen werden.",
                        .len = 119
                    };

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

  quoat *firstElement = (quoat *)llist_pull(my_list, 0);
  int diff = strncmp(firstElement->quoat, dummText, dummLen);
  if(diff){
    llist_free(my_list, qFreeSec);
  }else{
    llist_free(my_list, NULL);
  }
}

int addToList(char* text, int len){
    quoat *newQ = calloc(1, sizeof(quoat));
    newQ->quoat = calloc(len, sizeof(char));
    for(int i=0;i<len;i++){
        *(newQ->quoat+i) = *(text+i);
    }

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

int fileReader(char *fileName){
  // open file
  int lines = 0;

  FILE *qotd = fopen(fileName, "r");
  if(qotd){
    my_list = llist_create(NULL);

    if(my_list != NULL){
      char *saveBUF = calloc(1, sizeof(char));
      if( saveBUF != NULL ){

        int tmpLen = 100;
        char tmpBUF[tmpLen];

        int saveLen = 0;

        int tmpI = 0; // loop var
        int iter = 0; // iterration var
        int c = 0;
        while( (c=fgetc(qotd)) != EOF){
            tmpBUF[iter] = c;
            iter++;
            if(c == 10 || iter > tmpLen-1){
                if(iter == 1 && saveLen == 0) {iter=0;continue;} // empty line
                iter -= (c == 10 ? 1 : 0); // '\n'  don't save

                saveBUF = realloc(saveBUF, (saveLen+iter)*sizeof(char));
                if( saveBUF == NULL ){
                  DP("LL: realloc fail\n");
                  qFree();
                  lines = 0;
                  break;
                }
                for(tmpI=0;tmpI<=iter;tmpI++){
                    *(saveBUF+saveLen+tmpI)=*(tmpBUF+tmpI);
                }

                saveLen += iter;
                iter = 0;
                memset(tmpBUF, '\0', tmpLen);
                if(c == 10){
                    if(addToList(saveBUF, saveLen) != 0){

                      DP("LL: addition Fail\n");
                      qFree();
                      lines = 0;
                      break;
                    }
                    lines++;
                    memset(saveBUF, '\0', saveLen);
                    saveLen = 0;
                }
            }
        }

        DP("LL: saveLen = %u, last char = %u\n", saveLen, tmpBUF[iter-1]);
        free(saveBUF);
        fclose(qotd);
      } else {
          DP("LL: Cant alloce saveBUF\n");
      }
    } else {
        DP("LL: cant initial Linked list\n");
    }
  } else {
    DP("LL: Can't open the File\n");
  }

  if(lines == 0){
    DP("LL: Something was wrong with File Reading, Set Default\n");
    lines++;
    my_list = llist_create(NULL);
    if( my_list != NULL ){
      llist_push(my_list, (void *)&quoateDummy);
    }else{
      DP("LL: There is a big mistake!!!\n");
      qFree();
      lines=0;
    }
  }

  return lines;
}
