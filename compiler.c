#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.tab.h"
#include "compiler.h"
#include "buffer.h"
#include "list.h"
#include "symbol.h"

char *code;
int  pc;

static list *symbols;
extern FILE *yyin;
int yyparse();

void add_code(char *bytes, int length);

// TODO: Move this somewhere else
void set_error(const char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    vprintf(fmt, arg);
    printf("\n");
    
    va_end(arg);
}

/* declares */
void decl_byte(char *identifier, char value) {
    symbol *s;
    
    s = (symbol *)malloc(sizeof(symbol));
    memset(s, 0, sizeof(symbol));
    
    s->name = strdup(identifier);
    s->type = SYM_BYTE;
    
    /* No address, this symbol will be replaced during compile time */
    s->addr = 0;
    s->value.byte = value;
    
    list_add(symbols, s);
    
}

void decl_word(char *identifier, short value) {
    symbol *s;
    
    s = (symbol *)malloc(sizeof(symbol));
    memset(s, 0, sizeof(symbol));
    
    s->name = strdup(identifier);
    s->type = SYM_WORD;
    
    /* No address, this symbol will be replaced during compile time */
    s->addr = 0; 
    s->value.word = value;
    
    list_add(symbols, s);
}

void decl_string(char *identifier, char *value) {
    symbol *s;
    
    s = (symbol *)malloc(sizeof(symbol));
    memset(s, 0, sizeof(symbol));
    
    s->name = strdup(identifier);
    s->type = SYM_STR;
    s->addr = 0; // Will be calculated at the end
    s->value.str = strdup(value);
    
    list_add(symbols, s);
}

void decl_label(char *identifier) {
    symbol *s;
    
    s = (symbol *)malloc(sizeof(symbol));
    memset(s, 0, sizeof(symbol));
    
    s->name = strdup(identifier);
    s->type = SYM_LABEL;
    s->addr = pc;        // This is the next instruction
    
    s->name[strlen(s->name)-1] = '\0'; // Remove the ':' from the label
    
    list_add(symbols, s);
}


int get_reg_index(int r) {
    
    switch(r) {
        /* 8 bits */
        case REG_A:  return 0x07;
        case REG_B:  return 0x00;
        case REG_C:  return 0x01;
        case REG_D:  return 0x02;
        case REG_E:  return 0x03;
        case REG_H:  return 0x04;
        case REG_L:  return 0x05;
        
        /* 16 bits */
        case REG_BC: return 0x00;
        case REG_DE: return 0x01;
        case REG_HL: return 0x02;
        case REG_SP: return 0x03;
        case REG_AF: return 0x03;
    }
    
    return -1;
}

/* Opcodes */

void add_code(char *bytes, int length) {
    memcpy(&code[pc], bytes, length);
    pc += length;
    
    if(pc > MAX_MEMORY) {
        printf("Program is too big (> 64Kb). Cannot compile.\n");
        printf("Please check your code and make sure you need all of it!\n");
        printf("Aborting...\n");
        exit(-1);
    }
    
}

void org(int offset) {
    pc = offset;
}

/**
 * LD r, r'
 * LD A, I
 * LD A, R
 * LD I, A
 * LD R, A
 */
int ld_reg8_reg8(int r1, int r2) {
    char opc;
    
    /* Special cases */
    if((r1 == REG_A) && (r2 == REG_I)) {
        add_code("\xED\x57", 2);
        return C_OK;
    }
    
    if((r1 == REG_A) && (r2 == REG_R)) {
        add_code("\xED\x5F", 2);
        return C_OK;
    }
    
    if((r1 == REG_I) && (r2 == REG_A)) {
        add_code("\xED\x47", 2);
        return C_OK;
    }
    
    if((r1 == REG_R) && (r2 == REG_A)) {
        add_code("\xED\x4F", 2);
        return C_OK;
    }
    /* End special cases */
    
    if((r1 == REG_I) || (r1 == REG_R)) {
        set_error("Invalid left register");
        return C_ERROR;
    }
    
    if((r2 == REG_I) || (r2 == REG_R)) {
        set_error("Invalid right register");
        return C_ERROR;
    }
    
    r1 = get_reg_index(r1);
    r2 = get_reg_index(r2);
    
    opc = (1 << 6) | (r1 << 3) | r2;
    
    //buffer_append(code, &opc, 1);
    add_code(&opc, 1);
    
    printf("OPC: %02X\n", opc);

    return C_OK;
}

/**
 * LD r, n
 */
int ld_reg8_byte(int r1, char byte) {
    char opc[2];
    
    if((r1 == REG_I) || (r1 == REG_R)) {
        set_error("Invalid register");
        return C_ERROR;
    }
    
    r1 = get_reg_index(r1);
    
    opc[0] = (r1 << 3) | 0x06;
    opc[1] = byte;
    
    add_code(&opc[0], 2);
    
    return C_OK;
    
}

/**
 * LD r, (HL)
 * LD A, (BC)
 * LD A, (DE)
 */
int ld_reg8_preg16(int r1, int r2) {
    
    char opc;
    
    if((r1 == REG_I) || (r1 == REG_R)) {
        set_error("Invalid left register");
        return C_ERROR;
    }
    
    if(r2 == REG_HL) {
        r1 = get_reg_index(r1);
        opc = (1 << 6) | (r1 << 3) | 0x06;
        add_code(&opc, 1);
        
    } else if((r1 == REG_A) && (r2 == REG_BC)) {
        add_code("\x0A", 1);
        
    } else if((r1 == REG_A) && (r2 == REG_DE)) {
        add_code("\x1A", 1);
        
    } else {
        set_error("Invalid registers");
        return C_ERROR;
    }

    return C_OK;
}

/**
 * LD (IX+d), r
 * LD (IY+d), r
 */
int ld_reg8_preg16_byte(int r1, int r2, char byte) {
    
    char opc[3];
    
    if((r1 == REG_I) || (r1 == REG_R)) {
        set_error("Invalid left register");
        return C_ERROR;
    }
    
    if(r2 == REG_IX) {
        opc[0] = 0xDD;
    } else if(r2 == REG_IY) {
        opc[0] = 0xFD;
    } else {
        set_error("Invalid right register. IX or IY expected");
        return C_ERROR;
    }
    
    r1 = get_reg_index(r1);
    
    opc[1] = (1 << 6) | (r1 << 3) | 0x06;
    opc[2] = byte;
    add_code(&opc[0], 3);
    
    return C_OK;
}

/**
 * LD (HL), r
 * LD (BC), A
 * LD (DE), A
 */
int ld_preg16_reg8_byte(int r1, int r2) {
    
    char opc;
    
    if((r2 == REG_I) || (r2 == REG_R)) {
        set_error("Invalid right register");
        return C_ERROR;
    }
    
    if(r1 == REG_HL) {
        r2 = get_reg_index(r2);
    
        opc = (0x07 << 4) | r2;
        add_code(&opc, 1);
        
    } else if((r1 == REG_BC) && (r2 == REG_A)) {
        opc = 0x02;
        add_code(&opc, 1);
        
    } else if((r1 == REG_DE) && (r2 == REG_A)) {
        opc = 0x12;
        add_code(&opc, 1);
        
    } else {
        set_error("Invalid registers");
        return C_ERROR;
    }
    
    
    return C_OK;
    
    
}

/**
 * LD (IX+d), r
 * LD (IY+d), r
 */
int ld_preg16_byte_reg8(int r1, char byte, int r2) {
    
    char opc[3];
    
    if((r1 != REG_IX) && (r1 != REG_IY)) {
        set_error("Invalid left register. Expected IX or IY");
        return C_ERROR;
    }
    
    if((r2 == REG_I) || (r2 == REG_R)) {
        set_error("Invalid right register");
        return C_ERROR;
    }
    
    
    if(r1 == REG_IX) {
        opc[0] = 0xDD;
        
    } else if(r1 == REG_IY) {
        opc[0] = 0xFD;
        
    }
    
    r2 = get_reg_index(r2);
    
    opc[1] = (0x07 << 4) | r2;
    opc[2] = byte;
    
    add_code(&opc[0], 3);
    
    return C_OK;
    
}

/**
 * LD (HL), n
 */
int ld_preg16_byte(int r1, char byte) {
    
    char opc[2];
    
    if(r1 != REG_HL) {
        set_error("Invalid left register. (HL) expected");
        return C_ERROR;
    }
    
    opc[0] = 0x36;
    opc[1] = byte;
    
    add_code(&opc[0], 2);
    
    return C_OK;
    
}

/**
 * LD (IX+d), n
 * LD (IY+d), n
 */
int ld_preg16_byte_byte(int r1, char byte1, char byte2) {
    
    char opc[4];
    
    if((r1 != REG_IX) && (r1 != REG_IY)) {
        set_error("Invalid left register. Expected IX or IY");
        return C_ERROR;
    }
    
    if(r1 == REG_IX) {
        opc[0] = 0xDD;
        
        
    } else if(r1 == REG_IY) {
        opc[0] = 0xFD;
        
    }
        
    opc[1] = 0x36;
    opc[2] = byte1;
    opc[3] = byte2;
    
    add_code(&opc[0], 4);
    
    return C_OK;
    
}

/**
 * LD A, (nn)
 */
int ld_reg8_pword(int r1, short word) {
    
    char opc[3];
    
    if(r1 != REG_A) {
        // set_error("Invalid left register. Expected A");
        return C_ERROR;
    }
    
    opc[0] = 0x3A;
    opc[1] = word & 0x00FF;
    opc[2] = (word & 0xFF00) >> 8;
    
    add_code(&opc[0], 3);
    
    return C_OK;
    
}

/**
 * LD (nn), A
 */
int ld_pword_reg8(short word, int r1) {
    
    char opc[3];
    
    if(r1 != REG_A) {
        // set_error("Invalid right register. Expected A");
        return C_ERROR;
    }
    
    opc[0] = 0x32;
    opc[1] = word & 0x00FF;
    opc[2] = (word & 0xFF00) >> 8;
    
    add_code(&opc[0], 3);
    
    return C_OK;
    
}

/**
 * LD r, identifier
 * This will be translated to LD r, n
 */
int ld_reg8_identifier(int r1, char *identifier) {
    
    symbol search;
    symbol *s;
    
    search.name = identifier;
    
    s = list_find(symbols, &search);
    
    if(s == NULL) {
        set_error("Symbol %s not declared", identifier);
        return C_ERROR;
    }
    
    if(s->type != SYM_BYTE) {
        set_error("Symbol %s is not a BYTE type.", identifier);
        return C_ERROR;
    }
    
    return ld_reg8_byte(r1, s->value.byte);
}

/**
 * LD r, (identifier)
 * This will be translated to LD r, (nn)
 */
int ld_reg8_pidentifier(int r1, char *identifier) {
    
    symbol search;
    symbol *s;
    
    search.name = identifier;
    
    s = list_find(symbols, &search);
    
    if(s == NULL) {
        set_error("Symbol %s not declared", identifier);
        return C_ERROR;
    }
    
    if(s->type != SYM_WORD) {
        set_error("Symbol %s is not a WORD type.", identifier);
        return C_ERROR;
    }
    
    return ld_reg8_pword(r1, s->value.word);
}

/**
 * LD r16, nn
 * LD IX,  nn
 * LD IY,  nn
 */
int ld_reg16_word(int r1, short word) {
    
    char opc[4];
    
    if(r1 == REG_IX) {
        opc[0] = 0xDD;
        opc[1] = 0x21;
        opc[2] = (word & 0x00FF);
        opc[3] = (word & 0xFF00) >> 8;
        add_code(&opc[0], 4);
        
        return C_OK;
    }
    
    if(r1 == REG_IY) {
        opc[0] = 0xFD;
        opc[1] = 0x21;
        opc[2] = (word & 0x00FF);
        opc[3] = (word & 0xFF00) >> 8;
        add_code(&opc[0], 4);
        
        return C_OK;
    }
    
    r1 = get_reg_index(r1);
    opc[0] = (r1 << 4) | 0x01;
    opc[1] = (word & 0x00FF);
    opc[2] = (word & 0xFF00) >> 8;
    add_code(&opc[0], 3);
    
    return C_OK;
    
}

/**
 * LD r16, (nn)
 * LD IX,  (nn)
 * LD IY,  (nn)
 * LD HL,  (nn)
 */
int ld_reg16_pword(int r1, short word) {
    
    char opc[4];
    
    if(r1 == REG_HL) {
        opc[0] = 0x2A;
        opc[1] = (word & 0x00FF);
        opc[2] = (word & 0xFF00) >> 8;
        add_code(&opc[0], 3);
        
        return C_OK;
    }
    
    if(r1 == REG_IX) {
        opc[0] = 0xDD;
        opc[1] = 0x2A;
        opc[2] = (word & 0x00FF);
        opc[3] = (word & 0xFF00) >> 8;
        add_code(&opc[0], 4);
        
        return C_OK;
        
    }
    
    if(r1 == REG_IY) {
        opc[0] = 0xFD;
        opc[1] = 0x2A;
        opc[2] = (word & 0x00FF);
        opc[3] = (word & 0xFF00) >> 8;
        add_code(&opc[0], 4);
        
        return C_OK;
    }
    
    r1 = get_reg_index(r1);
    opc[0] = 0xED;
    opc[1] = (0x01 << 6) | (r1 << 4) | 0x0B;
    opc[2] = (word & 0x00FF);
    opc[3] = (word & 0xFF00) >> 8;
    add_code(&opc[0], 4);
    
    return C_OK;
    
}

/**
 * LD (nn), r16
 * LD (nn), HL
 * LD (nn), IX
 * LD (nn), IY
 */
int ld_pword_reg16(short word, int r1) {
    char opc[4];
    
    if(r1 == REG_HL) {
        opc[0] = 0x22;
        opc[1] = (word & 0x00FF);
        opc[2] = (word & 0xFF00) >> 8;
        add_code(&opc[0], 3);
        
        return C_OK;
        
    }
    
    if(r1 == REG_IX) {
        opc[0] = 0xDD;
        opc[1] = 0x22;
        opc[2] = (word & 0x00FF);
        opc[3] = (word & 0xFF00) >> 8;
        add_code(&opc[0], 4);
        
        return C_OK;
        
    }
    
    if(r1 == REG_IY) {
        opc[0] = 0xFD;
        opc[1] = 0x22;
        opc[2] = (word & 0x00FF);
        opc[3] = (word & 0xFF00) >> 8;
        add_code(&opc[0], 4);
        
        return C_OK;
        
    }
    
    r1 = get_reg_index(r1);
    opc[0] = 0xED;
    opc[1] = (0x01 << 6) | (r1 << 4) | 0x03;
    opc[2] = (word & 0x00FF);
    opc[3] = (word & 0xFF00) >> 8;
    add_code(&opc[0], 4);
    
    return C_OK;
    
}

/**
 * LD SP, HL
 * LD SP, IX
 * LD SP, IY
 */
int ld_reg16_reg16(int r1, int r2) {
    char opc[2];
    
    if(r1 == REG_SP) {
        if(r2 == REG_HL) {
            opc[0] = 0xF9;
            add_code(&opc[0], 1);
            
        } else if(r2 == REG_IX) {
            opc[0] = 0xDD;
            opc[1] = 0xF9;
            add_code(&opc[0], 2);
            
        } else if(r2 == REG_IY) {
            opc[0] = 0xFD;
            opc[1] = 0xF9;
            add_code(&opc[0], 2);
            
        } else {
            set_error("Right register expected to be HL, IX or IY");
            return C_ERROR;
        }
    } else {
        set_error("Left register expected to be SP");
        return C_ERROR;
    }
    
    return C_OK;
}

int ld_reg16_identifier(int r1, char *identifier) {
    symbol search;
    symbol *s;
    
    search.name = identifier;
    
    s = list_find(symbols, &search);
    
    if(s == NULL) {
        set_error("Symbol %s not declared", identifier);
        return C_ERROR;
    }
    
    if(s->type == SYM_WORD) {
        return ld_reg16_word(r1, s->value.word);
    }
    
    if((s->type == SYM_STR) || (s->type == SYM_LABEL)) {
        return ld_reg16_word(r1, s->addr);
    }
    
    set_error("Symbol %s is not a WORD or an ADDRESS (to a Label or String", 
              identifier);
    
    return C_ERROR;
}

/**
 * PUSH r16
 * PUSH IX
 * PUSH IY
 */
int push_reg16(int r1) {
    char opc[2];
    
    if(r1 == REG_IX) {
        opc[0] = 0xDD;
        opc[1] = 0xE5;
        add_code(&opc[0], 2);
        
        return C_OK;
    }
    
    if(r1 == REG_IY) {
        opc[0] = 0xFD;
        opc[1] = 0xE5;
        add_code(&opc[0], 2);
        
        return C_OK;
    }
    
    if(r1 == REG_SP) {
        set_error("Invalid register (SP).");
        
        return C_ERROR;
    }
    
    r1 = get_reg_index(r1);
    
    opc[0] = (0x03 << 6) | (r1 << 4) | 0x05;
    add_code(&opc[0], 1);
    
    return C_OK;
    
}

/**
 * POP r16
 * POP IX
 * POP IY
 */
int pop_reg16(int r1) {
    char opc[2];
    
    if(r1 == REG_IX) {
        opc[0] = 0xDD;
        opc[1] = 0xE1;
        add_code(&opc[0], 2);
        
        return C_OK;
    }
    
    if(r1 == REG_IY) {
        opc[0] = 0xDD;
        opc[1] = 0xFD;
        add_code(&opc[0], 2);
        
        return C_OK;
    }
    
    if(r1 == REG_SP) {
        set_error("Invalid register (SP).");
        
        return C_ERROR;
    }
    
    r1 = get_reg_index(r1);
    
    opc[0] = (0x03 << 6) | (r1 << 4) | 0x01;
    add_code(&opc[0], 1);
    
    return C_OK;
    
}

/**
 * EX DE, HL
 * EX AF,AF'
 */
int ex_reg16_reg16(int r1, int r2) {
    char opc;
    
    if((r1 == REG_DE) && (r2 == REG_HL)) {
        opc = 0xEB;
        add_code(&opc, 1);
        
    } else if((r1 == REG_AF) && (r2 == REG_AF2)) {
        opc = 0x08;
        add_code(&opc, 1);
        
    } else {
        set_error("Invalid register combination. Expected DE, HL or AF,AF'");
        
        return C_ERROR;
    }
    
    return C_OK;
}

/**
 * EX (SP), HL
 * EX (SP), IX
 * EX (SP), IY
 */
int ex_preg16_reg16(int r1, int r2) {
    char opc[2];
    
    if(r1 != REG_SP) {
        set_error("Invalid left register. Expected SP");
        
        return C_ERROR;
        
    }
    
    if(r2 == REG_HL) {
        opc[0] = 0xE3;
        add_code(&opc[0], 1);

    } else if (r2 == REG_IX) {
        opc[0] = 0xDD;
        opc[1] = 0xE3;
        add_code(&opc[0], 2);

    } else if (r2 == REG_IY) {
        opc[0] = 0xFD;
        opc[1] = 0xE3;
        add_code(&opc[0], 2);

    } else {
        set_error("Invalid right register. Expected HL, IX or IY");
        
        return C_ERROR;
    }
    
    return C_OK;
    
}

/**
 * EXX
 */
int exx() {
    char opc;
    
    opc = 0xD9;
    add_code(&opc, 1);
    
    return C_OK;
}

/**
 * LDI
 */
int ldi() {
    char opc[2];
    
    opc[0] = 0xED;
    opc[1] = 0xA0;
    add_code(&opc[0], 2);
    
    return C_OK;
}

/**
 * LDIR
 */
int ldir() {
    char opc[2];
    
    opc[0] = 0xED;
    opc[1] = 0xB0;
    add_code(&opc[0], 2);
    
    return C_OK;
}

/**
 * LDD
 */
int ldd() {
    char opc[2];
    
    opc[0] = 0xED;
    opc[1] = 0xA8;
    add_code(&opc[0], 2);
    
    return C_OK;
}

/**
 * LDDR
 */
int lddr() {
    char opc[2];
    
    opc[0] = 0xED;
    opc[1] = 0xB8;
    add_code(&opc[0], 2);
    
    return C_OK;
}

/**
 * CPI
 */
int cpi() {
    char opc[2];
    
    opc[0] = 0xED;
    opc[1] = 0xA1;
    add_code(&opc[0], 2);
    
    return C_OK;
}

/**
 * CPIR
 */
int cpir() {
    char opc[2];
    
    opc[0] = 0xED;
    opc[1] = 0xB1;
    add_code(&opc[0], 2);
    
    return C_OK;
}

/**
 * CPD
 */
int cpd() {
    char opc[2];
    
    opc[0] = 0xED;
    opc[1] = 0xA9;
    add_code(&opc[0], 2);
    
    return C_OK;
}

/**
 * CPDR
 */
int cpdr() {
    char opc[2];
    
    opc[0] = 0xED;
    opc[1] = 0xB9;
    add_code(&opc[0], 2);
    
    return C_OK;
}

/* Start here */

void usage(char *pgm_name) {
    printf("Usage: %s <file.zasm>\n", pgm_name);
}

void build_binary(char *file_name) {
    FILE *f;
    
    f = fopen(file_name, "w+");
    if(f == NULL) {
        printf("Cannot create output file %s. Aborting...\n", file_name);
        return;
    }
    
    fwrite(code, 1, MAX_MEMORY, f);
    fclose(f);
    
}

int main(int argc, char **argv) {
    
    int error = 0;
    FILE *f;
    
    
    if(argc < 2) {
        usage(argv[0]);
        return -1;
    }
    
    f = fopen(argv[1], "r");
    if(f == NULL) {
        printf("Unable to open file %s\nAborting...", argv[1]);
        return -1;
    }
    
    yyin = f;
    //code  = buffer_new(0);
    code = (char *)malloc(MAX_MEMORY);
    symbols = list_new(symbol_free, symbol_cmp);

    memset(code, 0, MAX_MEMORY);
    pc = 0;

    while(!feof(yyin)) {
        if(yyparse() != 0) {
            error = 1;
        }
    }

    if(!error) {
        build_binary("result.bin");
    }

    //buffer_free(code);
    free(code);
    list_free(symbols);
    fclose(f);
    
    return 0;
}
