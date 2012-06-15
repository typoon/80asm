#ifndef SYMBOL_H_
#define SYMBOL_H_


#define SYM_STR     0x01
#define SYM_BYTE    0x02
#define SYM_WORD    0x03
#define SYM_LABEL   0x04

typedef struct _symbol {
    char *name;
    int   type;
    short addr;
    
    union {
        char  *str;
        char   byte;
        short  word;
    } value;
} symbol;


void symbol_free(void *sym);
int symbol_cmp(void *sym1, void *sym2);

#endif
