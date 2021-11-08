#ifndef _LLIST_H
#define _LLIST_H

/* llist.h
 * Generic Linked List
 */

 // from https://gist.github.com/meylingtaing/11018042

struct node {
    void *key;
    int keyLen;
    void *data;
    struct node *next;
};

typedef struct node * llist;

/* llist_create: Create a linked list */
llist *llist_create();

/* llist_free: Free a linked list */
void llist_free(llist *list, void (*freeD)(void *data));

/* llist_free: Free a linked list node */
void llist_freeNode(llist *list, void (*freeD)(void *data), void* key, int keyLen);

/* llist_push: Add to head of list */
int llist_push(llist *list, void *data, void* key, int keyLen);

/* llist_pop: remove and return head of linked list */
//void *llist_pop(llist *list);

/* llist_pull: return nr element of linked list */
void *llist_pull(llist *list, void* key, int keyLen);

/* llist_print: print linked list */
void llist_print(llist *list, void (*print)(void *data));

#endif /* _LLIST_H */
