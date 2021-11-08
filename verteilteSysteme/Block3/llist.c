/* llist.c
 * Generic Linked List implementation
 */

// from https://gist.github.com/meylingtaing/11018042

#include <stdlib.h>
#include <stdio.h>
#include "llist.h"

llist *llist_create(void *new_data)
{
    struct node *new_node;

    llist *new_list = (llist *)malloc(sizeof (llist));
    if( new_list == NULL ){
      return (llist *)NULL;
    }
    *new_list = (struct node *)malloc(sizeof (struct node));
    if( new_list == NULL ){
      return (llist *)NULL;
    }

    new_node = *new_list;
    new_node->data = new_data;
    new_node->next = NULL;
    return new_list;
}
//void llist_print(llist *list, void (*print)(void *))
void llist_free(llist *list, void (*freeD)(void *data))
{
    struct node *curr = *list;
    struct node *next;

    while (curr != NULL) {
        if(freeD != NULL){
            freeD(curr->data);
        }
        next = curr->next;
        free(curr);
        curr = next;
    }

    free(list);
}

// Returns 0 on failure
int llist_add_inorder(void *data, llist *list,
                       int (*comp)(void *, void *))
{
    struct node *new_node;
    struct node *curr;
    struct node *prev = NULL;

    if (list == NULL || *list == NULL) {
        fprintf(stderr, "llist_add_inorder: list is null\n");
        return 0;
    }

    curr = *list;
    if (curr->data == NULL) {
        curr->data = data;
        return 1;
    }

    new_node = (struct node *)malloc(sizeof (struct node));
    if( new_node == NULL ){ return 0; }
    new_node->data = data;

    // Find spot in linked list to insert new node
    while (curr != NULL && curr->data != NULL && comp(curr->data, data) < 0) {
        prev = curr;
        curr = curr->next;
    }
    new_node->next = curr;

    if (prev == NULL)
        *list = new_node;
    else
        prev->next = new_node;

    return 1;
}

int llist_push(llist *list, void *data)
{
    struct node *head ;
    struct node *new_node;
    if (list == NULL || *list == NULL) {
        fprintf(stderr, "llist_add_inorder: list is null\n");
    }

    head = *list;

    while(head->next != NULL){
        head = head->next;
    }

    // Head is empty node
    if (head->data == NULL){
        head->data = data;
    }
    // Head is not empty, add new node to front
    else {
        new_node = malloc(sizeof (struct node));
        if( new_node == NULL ){ return 1; }
        new_node->data = data;
        new_node->next = NULL;
        head->next = new_node;
    }
    return 0;
}

void *llist_pop(llist *list)
{
    void *popped_data;
    struct node *head = *list;

    if (list == NULL || head->data == NULL)
        return NULL;

    popped_data = head->data;
    *list = head->next;

    free(head);

    return popped_data;
}

void *llist_pull(llist *list, int nr){

    struct node *curr = *list;

    if (curr == NULL)
        {return NULL;}

    int i = 0;

    while(1){
        if( i > nr ){
          fprintf(stderr, "llist_pull E1\n");
          return NULL;
        }
        if( i == nr ){
          return (void *)curr->data;
        }
        if( curr->next == NULL ){
          return NULL;
        } else {
          curr = curr->next;
        }
        i++;
    }
}

void llist_print(llist *list, void (*print)(void *))
{
    struct node *curr = *list;
    while (curr != NULL) {
        print(curr->data);
        curr = curr->next;
    }
}
