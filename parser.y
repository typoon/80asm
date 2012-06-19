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

%%

start:
    org body
    | body
    ;

org:
      ORG WORD                         { org($2); }
    | ORG BYTE                         { org($2); }

body:
    body declares mnemonics
    | declares mnemonics
    ;
    
declares:
    declares declare
    | declare
    ;

declare:
      DECL_BYTE IDENTIFIER BYTE        { decl_byte($2, $3);   free($2); }
    | DECL_WORD IDENTIFIER WORD        { decl_word($2, $3);   free($2); }
    | DECL_STRING IDENTIFIER STRING    { decl_string($2, $3); free($2); free($3); }
    | DECL_LABEL                       { decl_label($1);      free($1); }
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
    
    ;

%%



static void yyerror(const char *s) {
    printf("Error: %s on line %d\n", s, line);
}
