#ifndef COMPILER_H_
#define COMPILER_H_

#define C_ERROR -1
#define C_OK     0

#define MAX_MEMORY ((1024 * 64) - 1)  /* Z80 has only 16 bit address bus */

/* declares: */
void decl_byte(char *identifier, char value);
void decl_word(char *identifier, short value);
void decl_string(char *identifier, char *value);
void decl_label(char *identifier);
void org(int offset);

/* Opcodes */

/* ld 8 bits */
int ld_reg8_reg8(int r1, int r2);
int ld_reg8_byte(int r1, char byte);
int ld_reg8_preg16(int r1, int r2);
int ld_reg8_preg16_byte(int r1, int r2, char byte);
int ld_preg16_reg8_byte(int r1, int r2);
int ld_preg16_byte_reg8(int r1, char byte, int r2);
int ld_preg16_byte(int r1, char byte);
int ld_preg16_byte_byte(int r1, char byte1, char byte2);
int ld_reg8_pword(int r1, short word);
int ld_pword_reg8(short word, int r1);
int ld_reg8_identifier(int r1, char *identifier);
int ld_reg8_pidentifier(int r1, char *identifier);
    
#endif
