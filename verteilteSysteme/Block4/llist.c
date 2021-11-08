/* llist.c
 * Generic Linked List implementation
 */

// from https://gist.github.com/meylingtaing/11018042

#include <stdlib.h>
#include <stdio.h>
#include "llist.h"

llist *llist_create()
{
    struct node *new_node = NULL;

    llist *new_list = (llist *)calloc(1,sizeof(llist));
    if( new_list == NULL ){
      return (llist *)NULL;
    }
    *new_list = (struct node *)calloc(1,sizeof(struct node));
    if( new_list == NULL ){
      return (llist *)NULL;
    }

    new_node = *new_list;
    new_node->next = NULL;
    return new_list;
}

void llist_free(llist *list, void (*freeD)(void *data))
{
    struct node *curr = *list;
    struct node *next;

    while (curr != NULL) {
        if(freeD != NULL){
            freeD(curr->data);
        }
        if(curr->key != NULL){
            free(curr->key);
        }
        next = curr->next;
        free(curr);
        curr = next;
    }

    free(list);
}

int llist_push(llist *list, void *data, void* key, int keyLen){
    struct node *head ;
    struct node *new_node;
    if (list == NULL || *list == NULL) {
        fprintf(stderr, "llist_push: list is null\n");
        return 0;
    }
    if( data == NULL || key == NULL ){
        fprintf(stderr, "llist_push: data or key is null\n");
        return 0;
    }

    head = *list;

    while(head->next != NULL){
        head = head->next;
    }

    // Head is empty node
    if (head->data == NULL){
        head->key = key;
        head->keyLen = keyLen;
        head->data = data;
    }
    // Head is not empty, add new node to front
    else {
        new_node = calloc(1,sizeof(struct node));
        if( new_node == NULL ){ return 1; }
        new_node->key = key;
        new_node->keyLen = keyLen;
        new_node->data = data;
        new_node->next = NULL;
        head->next = new_node;
    }
    return 0;
}

struct node* llist_searchNode(llist *list, void* key, int keyLen){
    struct node *curr = *list;

    if (curr == NULL){
        return NULL;
    }

    int keyCheck = 0;

    while(curr != NULL){
        if(curr->keyLen == keyLen){
            keyCheck = 0;
            for(int i=0;i<keyLen;i++){
                if(*((char*)(curr->key+i)) != *((char*)(key+i))){
                    keyCheck++;
                    break;
                }
            }
            if(keyCheck == 0){
                break;
            }
        }
        curr = curr->next;
    }
    return curr;
}
void llist_freeNode(llist *list, void (*freeD)(void *data), void* key, int keyLen){
    if (list == NULL){
        return;
    }

    struct node *curr = *list;
    struct node *pre = *list;
    int keyCheck = 0;

    while(curr != NULL){
        if(curr->keyLen == keyLen){
            keyCheck = 0;
            for(int i=0;i<keyLen;i++){
                if(*((char*)(curr->key+i)) != *((char*)(key+i))){
                    keyCheck++;
                    break;
                }
            }
            if(keyCheck == 0){
                break;
            }
        }
        pre = curr;
        curr = curr->next;
    }

    if(curr != NULL){
        if(*list != curr){
            pre->next = curr->next;
            if(curr->data != NULL)freeD(curr);
            free(curr->key);
            free(curr);
        } else {
            if(curr->data != NULL)freeD(curr);
            free(curr->key);
            (*list)->next = NULL;
            (*list)->key = NULL;
            (*list)->data = NULL;
            (*list)->keyLen = -1;
        }
    }

}
void *llist_pull(llist *list, void* key, int keyLen){

    if (list == NULL){
        return NULL;
    }

    struct node * tmp = llist_searchNode(list, key, keyLen);

    return tmp->data;
}

void llist_print(llist *list, void (*print)(void *))
{
    struct node *curr = *list;
    while (curr != NULL) {
        print(curr->data);
        curr = curr->next;
    }
}
