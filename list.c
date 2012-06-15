#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "list.h"

list* list_new(list_element_free free_fn, list_element_cmp cmp_fn)
{
    list *l = (list *)malloc(sizeof(list));
    l->free_fn = free_fn;
    l->cmp_fn = cmp_fn;
    l->elements = NULL;
    
    return l;
}

void list_add(list *l, void *element)
{
    list_element *e;
    
    if(l == NULL) {
        return;
    }
    
    if(l->elements == NULL) {
        l->elements = (list_element *)malloc(sizeof(list_element));
        assert(l->elements);
        
        l->elements->next = NULL;
        l->elements->v = element;
    } else {
        e = l->elements;
        while(e->next != NULL) {
            e = e->next;
        }
        
        e->next = (list_element *)malloc(sizeof(list_element));
        assert(e->next);
        
        e = e->next;
        e->next = NULL;
        e->v = element;
    }
}

void list_free(list *l)
{
    list_element *e;
    list_element *f;
    
    e = l->elements;
    while(e) {
        l->free_fn(e->v);
        f = e;
        e = e->next;
        free(f);
    }
    
    free(l);
}

void* list_find(list *l, void *what)
{
    list_element *e;
    
    e = l->elements;
    
    if(e == NULL) {
        return NULL;
    }
    
    while(e != NULL) {
        if(l->cmp_fn(e->v, what) == 0) {
            return e->v;
        }
        
        e = e->next;
    }
    
    return NULL;
}
