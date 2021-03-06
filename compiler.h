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

/* ld 16 bits */
int ld_reg16_word(int r1, short word);
int ld_reg16_pword(int r1, short word);
int ld_pword_reg16(short word, int r1);
int ld_reg16_reg16(int r1, int r2);
int ld_reg16_identifier(int r1, char *identifier);

/* push */
int push_reg16(int r1);

/* pop */
int pop_reg16(int r1);

/* exchange */
int ex_reg16_reg16(int r1, int r2);
int ex_preg16_reg16(int r1, int r2);
int exx();

/* block transfer */
int ldi();
int ldir();
int ldd();
int lddr();
int cpi();
int cpir();
int cpd();
int cpdr();

/* arithmetic */
int add_reg8_reg8(int r1, int r2);
int add_reg8_byte(int r1, char byte);
int add_reg8_preg16(int r1, int r2);
int add_reg8_preg16_byte(int r1, int r2, char byte);
int add_reg8_identifier(int r1, char *identifier);

int adc_reg8_reg8(int r1, int r2);
int adc_reg8_byte(int r1, char byte);
int adc_reg8_preg16(int r1, int r2);
int adc_reg8_preg16_byte(int r1, int r2, char byte);
int adc_reg8_identifier(int r1, char *identifier);

int sub_reg8(int r);
int sub_byte(char byte);
int sub_preg16(int r);
int sub_preg16_byte(int r, char byte);
int sub_identifier(char *identifier);

int sbc_reg8(int r);
int sbc_byte(char byte);
int sbc_preg16(int r);
int sbc_preg16_byte(int r, char byte);
int sbc_identifier(char *identifier);

/* logic */
int and_reg8(int r);
int and_byte(char byte);
int and_preg16(int r);
int and_preg16_byte(int r, char byte);
int and_identifier(char *identifier);

int or_reg8(int r);
int or_byte(char byte);
int or_preg16(int r);
int or_preg16_byte(int r, char byte);
int or_identifier(char *identifier);

int xor_reg8(int r);
int xor_byte(char byte);
int xor_preg16(int r);
int xor_preg16_byte(int r, char byte);
int xor_identifier(char *identifier);

int cp_reg8(int r);
int cp_byte(char byte);
int cp_preg16(int r);
int cp_preg16_byte(int r, char byte);
int cp_identifier(char *identifier);

int inc_reg8(int r);
int inc_preg16(int r);
int inc_preg16_byte(int r, char byte);

int dec_reg8(int r);
int dec_preg16(int r);
int dec_preg16_byte(int r, char byte);

/* General purpose */
int daa();
int cpl();
int neg();
int ccf();
int scf();
int nop();
int halt();
int di();
int ei();
int im(int byte);

#endif
