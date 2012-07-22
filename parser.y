%{
#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

static void yyerror(const char *s);
int yylex();
int yyparse();
extern int line;


%}

%union {
    char     byte_val;
    short    word_val;
    char    *str_val;
    char    *identifier;
    int      reg8;
    int      reg16;
    char    *label;
}

%token <byte_val>    BYTE
%token <word_val>    WORD
%token <str_val>     STRING
%token <identifier>  IDENTIFIER
%token <reg8>        REG8
%token <reg16>       REG16

%token <label> DECL_LABEL
%token DECL_BYTE
%token DECL_WORD
%token DECL_STRING
%token ORG

%token LEFT_PAR
%token RIGHT_PAR
%token COMMA
%token PLUS

%token REG_A
%token REG_B
%token REG_C
%token REG_D
%token REG_E
%token REG_H
%token REG_L
%token REG_F
%token REG_R
%token REG_I
%token REG_IX
%token REG_IY
%token REG_SP
%token REG_BC
%token REG_DE
%token REG_HL
%token REG_AF
%token REG_AF2


%token OPC_LD
%token OPC_PUSH
%token OPC_POP
%token OPC_EX
%token OPC_EXX
%token OPC_LDI
%token OPC_LDIR
%token OPC_LDD
%token OPC_LDDR
%token OPC_CPI
%token OPC_CPIR
%token OPC_CPD
%token OPC_CPDR
%token OPC_ADD
%token OPC_ADC
%token OPC_SUB
%token OPC_SBC
%token OPC_AND
%token OPC_OR

%token DATA
%token CODE

%%

start:
    data code
    | code
    ;

data:
    DATA declares
    ;

code:
    CODE org body
    | CODE body
    ;

org:
      ORG WORD                         { org($2); }
    | ORG BYTE                         { org($2); }

body:
    body labels mnemonics
    | labels mnemonics
    | mnemonics
    ;

labels:
    DECL_LABEL                         { decl_label($1); free($1); }
    ;

declares:
    declares declare
    | declare
    ;

declare:
      DECL_BYTE IDENTIFIER BYTE        { decl_byte($2, $3);   free($2); }
    | DECL_WORD IDENTIFIER WORD        { decl_word($2, $3);   free($2); }
    | DECL_STRING IDENTIFIER STRING    { decl_string($2, $3); free($2); free($3); }
    ;

mnemonics:
    mnemonics opcodes
    | opcodes
    ;

opcodes:
    ld
    | push
    | pop
    | ex
    | exx
    | ldi
    | ldir
    | ldd
    | lddr
    | cpi
    | cpir
    | cpd
    | cpdr
    | add
    | adc
    | sub
    | sbc
    | and
    | or
    ;

ld:
      OPC_LD REG8 COMMA REG8                                   { if(ld_reg8_reg8($2, $4)   < 0) YYABORT; }
    | OPC_LD REG8 COMMA BYTE                                   { if(ld_reg8_byte($2, $4)   < 0) YYABORT; }
    | OPC_LD REG8 COMMA LEFT_PAR REG16 RIGHT_PAR               { if(ld_reg8_preg16($2, $5) < 0) YYABORT; }
    | OPC_LD REG8 COMMA LEFT_PAR REG16 PLUS BYTE RIGHT_PAR     { if(ld_reg8_preg16_byte($2, $5, $7) < 0) YYABORT; }
    | OPC_LD LEFT_PAR REG16 RIGHT_PAR COMMA REG8               { if(ld_preg16_reg8_byte($3, $6) < 0) YYABORT; }
    | OPC_LD LEFT_PAR REG16 PLUS BYTE RIGHT_PAR COMMA REG8     { if(ld_preg16_byte_reg8($3, $5, $8) < 0) YYABORT; }
    | OPC_LD LEFT_PAR REG16 RIGHT_PAR COMMA BYTE               { if(ld_preg16_byte($3, $6) < 0) YYABORT; }
    | OPC_LD LEFT_PAR REG16 PLUS BYTE RIGHT_PAR COMMA BYTE     { if(ld_preg16_byte_byte($3, $5, $8) < 0) YYABORT; } 
    | OPC_LD REG8 COMMA LEFT_PAR WORD RIGHT_PAR                { if(ld_reg8_pword($2, $5) < 0) YYABORT; }
    | OPC_LD LEFT_PAR WORD RIGHT_PAR COMMA REG8                { if(ld_pword_reg8($3, $6) < 0) YYABORT; }
    | OPC_LD REG8 COMMA IDENTIFIER                             { if(ld_reg8_identifier($2, $4) < 0) { free($4); YYABORT; } free($4); }
    | OPC_LD REG8 COMMA LEFT_PAR IDENTIFIER RIGHT_PAR          { if(ld_reg8_pidentifier($2, $5) < 0) { free($5); YYABORT; } free($5); }
    | OPC_LD REG16 COMMA WORD                                  { if(ld_reg16_word($2, $4)  < 0) YYABORT; }
    | OPC_LD REG16 COMMA LEFT_PAR WORD RIGHT_PAR               { if(ld_reg16_pword($2, $5) < 0) YYABORT; }
    | OPC_LD LEFT_PAR WORD RIGHT_PAR COMMA REG16               { if(ld_pword_reg16($3, $6) < 0) YYABORT; }
    | OPC_LD REG16 COMMA REG16                                 { if(ld_reg16_reg16($2, $4) < 0) YYABORT; }
    | OPC_LD REG16 COMMA IDENTIFIER                            { if(ld_reg16_identifier($2,$4) < 0) { free($4); YYABORT; } free($4); }
    ;

push:
    OPC_PUSH REG16                                             { if(push_reg16($2) < 0) YYABORT; }
    ;

pop:
    OPC_POP REG16                                              { if(pop_reg16($2) < 0) YYABORT; }
    ;

ex:
    OPC_EX REG16 COMMA REG16                                   { if(ex_reg16_reg16($2, $4) < 0) YYABORT; }
    | OPC_EX LEFT_PAR REG16 RIGHT_PAR COMMA REG16              { if(ex_preg16_reg16($3, $6) < 0) YYABORT; }

exx:
    OPC_EXX                                                    { if(exx() < 0) YYABORT; }
    ;

ldi:
    OPC_LDI                                                    { if(ldi() < 0) YYABORT; }
    ;
    
ldir:
    OPC_LDIR                                                   { if(ldir() < 0) YYABORT; }
    ;
    
ldd:
    OPC_LDD                                                    { if(ldd() < 0) YYABORT; }
    ;
    
lddr:
    OPC_LDDR                                                   { if(lddr() < 0) YYABORT; }
    ;
    
cpi:
    OPC_CPI                                                    { if(cpi() < 0) YYABORT; }
    ;
    
cpir:
    OPC_CPIR                                                   { if(cpir() < 0) YYABORT; }
    ;
    
cpd:
    OPC_CPD                                                    { if(cpd() < 0) YYABORT; }
    ;
    
cpdr:
    OPC_CPDR                                                   { if(cpdr() < 0) YYABORT; }
    ;

add:
    OPC_ADD REG8 COMMA REG8                                    { if(add_reg8_reg8($2, $4) < 0) YYABORT; }
    | OPC_ADD REG8 COMMA BYTE                                  { if(add_reg8_byte($2, $4) < 0) YYABORT; }
    | OPC_ADD REG8 COMMA LEFT_PAR REG16 RIGHT_PAR              { if(add_reg8_preg16($2, $5) < 0) YYABORT; }
    | OPC_ADD REG8 COMMA LEFT_PAR REG16 PLUS BYTE RIGHT_PAR    { if(add_reg8_preg16_byte($2, $5, $7) < 0) YYABORT; }
    | OPC_ADD REG8 COMMA IDENTIFIER                            { if(add_reg8_identifier($2, $4) < 0) { free($4); YYABORT; } free($4); }
    
    | OPC_ADD REG8                                             { if(add_reg8_reg8(REG_A, $2) < 0) YYABORT; }
    | OPC_ADD BYTE                                             { if(add_reg8_byte(REG_A, $2) < 0) YYABORT; }
    | OPC_ADD LEFT_PAR REG16 RIGHT_PAR                         { if(add_reg8_preg16(REG_A, $3) < 0) YYABORT; }
    | OPC_ADD LEFT_PAR REG16 PLUS BYTE RIGHT_PAR               { if(add_reg8_preg16_byte(REG_A, $3, $5) < 0) YYABORT; }
    | OPC_ADD IDENTIFIER                                       { if(add_reg8_identifier(REG_A, $2) < 0) { free($2); YYABORT; } free($2); }
    
    ;

adc:
    OPC_ADC REG8 COMMA REG8                                    { if(adc_reg8_reg8($2, $4) < 0) YYABORT; }
    | OPC_ADC REG8 COMMA BYTE                                  { if(adc_reg8_byte($2, $4) < 0) YYABORT; }
    | OPC_ADC REG8 COMMA LEFT_PAR REG16 RIGHT_PAR              { if(adc_reg8_preg16($2, $5) < 0) YYABORT; }
    | OPC_ADC REG8 COMMA LEFT_PAR REG16 PLUS BYTE RIGHT_PAR    { if(adc_reg8_preg16_byte($2, $5, $7) < 0) YYABORT; }
    | OPC_ADC REG8 COMMA IDENTIFIER                            { if(adc_reg8_identifier($2, $4) < 0) { free($4); YYABORT; } free($4); }

    | OPC_ADC REG8                                             { if(adc_reg8_reg8(REG_A, $2) < 0) YYABORT; }
    | OPC_ADC BYTE                                             { if(adc_reg8_byte(REG_A, $2) < 0) YYABORT; }
    | OPC_ADC LEFT_PAR REG16 RIGHT_PAR                         { if(adc_reg8_preg16(REG_A, $3) < 0) YYABORT; }
    | OPC_ADC LEFT_PAR REG16 PLUS BYTE RIGHT_PAR               { if(adc_reg8_preg16_byte(REG_A, $3, $5) < 0) YYABORT; }
    | OPC_ADC IDENTIFIER                                       { if(adc_reg8_identifier(REG_A, $2) < 0) { free($2); YYABORT; } free($2); }
    ;

sub:
    OPC_SUB REG8                                               { if(sub_reg8($2) < 0) YYABORT; }
    | OPC_SUB BYTE                                             { if(sub_byte($2) < 0) YYABORT; }
    | OPC_SUB LEFT_PAR REG16 RIGHT_PAR                         { if(sub_preg16($3) < 0) YYABORT; }
    | OPC_SUB LEFT_PAR REG16 PLUS BYTE RIGHT_PAR               { if(sub_preg16_byte($3, $5) < 0) YYABORT; }
    | OPC_SUB IDENTIFIER                                       { if(sub_identifier($2) < 0) { free($2); YYABORT; } free($2); }
    ;

sbc:
    OPC_SBC REG8                                               { if(sbc_reg8($2) < 0) YYABORT; }
    | OPC_SBC BYTE                                             { if(sbc_byte($2) < 0) YYABORT; }
    | OPC_SBC LEFT_PAR REG16 RIGHT_PAR                         { if(sbc_preg16($3) < 0) YYABORT; }
    | OPC_SBC LEFT_PAR REG16 PLUS BYTE RIGHT_PAR               { if(sbc_preg16_byte($3, $5) < 0) YYABORT; }
    | OPC_SBC IDENTIFIER                                       { if(sbc_identifier($2) < 0) { free($2); YYABORT; } free($2); }
    ;

and:
    OPC_AND REG8                                               { if(and_reg8($2) < 0) YYABORT; }
    | OPC_AND BYTE                                             { if(and_byte($2) < 0) YYABORT; }
    | OPC_AND LEFT_PAR REG16 RIGHT_PAR                         { if(and_preg16($3) < 0) YYABORT; }
    | OPC_AND LEFT_PAR REG16 PLUS BYTE RIGHT_PAR               { if(and_preg16_byte($3, $5) < 0) YYABORT; }
    | OPC_AND IDENTIFIER                                       { if(and_identifier($2) < 0) { free($2); YYABORT; } free($2); }
    ;

or:
    OPC_OR REG8                                                { if(or_reg8($2) < 0) YYABORT; }
    | OPC_OR BYTE                                              { if(or_byte($2) < 0) YYABORT; }
    | OPC_OR LEFT_PAR REG16 RIGHT_PAR                          { if(or_preg16($3) < 0) YYABORT; }
    | OPC_OR LEFT_PAR REG16 PLUS BYTE RIGHT_PAR                { if(or_preg16_byte($3, $5) < 0) YYABORT; }
    | OPC_OR IDENTIFIER                                        { if(or_identifier($2) < 0) { free($2); YYABORT; } free($2); }
    ;

%%



static void yyerror(const char *s) {
    printf("Error: %s on line %d\n", s, line);
}
