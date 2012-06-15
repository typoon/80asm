#ifndef LIST_H_
#define LIST_H_



typedef void (*list_element_free)(void *v);

// This function shall return 0 if v1 and v2 are equal, -1 otherwise
typedef int  (*list_element_cmp)(void *v1, void *v2);

typedef struct _list_element {
    void *v;
    struct _list_element *next;
} list_element;

typedef struct _list {
    list_element_free free_fn;
    list_element_cmp  cmp_fn;
    list_element *elements;  
} list;

list* list_new(list_element_free free_fn, list_element_cmp cmp_fn);
void list_add(list *l, void *element);
void list_free(list *l);
void* list_find(list *l, void *what);

#endif
