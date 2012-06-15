#include <stdlib.h>
#include <string.h>
#include "symbol.h"
#include "list.h"

void symbol_free(void *sym)
{
    symbol *s = (symbol *)sym;
    
    if(!s) {
        return;
    }
    
    if(s->name) {
        free(s->name);
    }
    
    if(s->type == SYM_STR) {
        if(s->value.str) {
            free(s->value.str);
        }
    }

    free(s);
}

int symbol_cmp(void *sym1, void *sym2)
{
    symbol *s1, *s2;
    
    s1 = sym1;
    s2 = sym2;
    
    if(strcmp(s1->name, s2->name) == 0) {
        return 0;
    }
    
    return -1;
    
}
