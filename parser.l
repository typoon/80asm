%{
#include <stdio.h>
#include "parser.tab.h"

#define YY_NO_INPUT

int new_instruction = 0;
int line = 1;

%}

%option nounput
%option noyywrap

%x COMMENT

%%


<INITIAL>{
    [\r\t]                      ;
    [\n]                        { ++line; }

    "A"                         { yylval.reg8  = REG_A;      return REG8; }
    "B"                         { yylval.reg8  = REG_B;      return REG8; }
    "C"                         { yylval.reg8  = REG_C;      return REG8; }
    "D"                         { yylval.reg8  = REG_D;      return REG8; }
    "E"                         { yylval.reg8  = REG_E;      return REG8; }
    "H"                         { yylval.reg8  = REG_H;      return REG8; }
    "L"                         { yylval.reg8  = REG_L;      return REG8; }
    "F"                         { yylval.reg8  = REG_F;      return REG8; }
    "R"                         { yylval.reg8  = REG_R;      return REG8; }
    "I"                         { yylval.reg8  = REG_I;      return REG8; }
    "BC"                        { yylval.reg16 = REG_BC;     return REG16; }
    "DE"                        { yylval.reg16 = REG_DE;     return REG16; }
    "HL"                        { yylval.reg16 = REG_HL;     return REG16; }
    "IX"                        { yylval.reg16 = REG_IX;     return REG16; }
    "IY"                        { yylval.reg16 = REG_IY;     return REG16; }
    "SP"                        { yylval.reg16 = REG_SP;     return REG16; }
    "AF'"                       { yylval.reg16 = REG_AF2;    return REG16; }
    "AF"                        { yylval.reg16 = REG_AF;     return REG16; }

    

    ","                         { return COMMA; }
    ".byte"                     { return DECL_BYTE;   }
    ".word"                     { return DECL_WORD;   }
    ".string"                   { return DECL_STRING; }
    ".org"                      { return ORG; }
    [a-zA-Z]+":"                { yylval.identifier = strdup(yytext); return DECL_LABEL; }
    

    "+"                         { return PLUS;        }
    "("                         { return LEFT_PAR;    }
    ")"                         { return RIGHT_PAR;   }

    
    [\$][a-zA-Z]+[a-zA-Z0-9]*   { yylval.identifier = strdup(yytext); return IDENTIFIER; }

    ["]([^\\"]+|\\.)*["]        { yylval.str_val = strdup(yytext); return STRING; }



    [1-9]+[0-9]*                { 
                                    int x = atoi(yytext); 
                                    if((x >=0 )&& (x <= 255)) {
                                        yylval.byte_val = x;
                                        return BYTE;
                                    } else {
                                        yylval.word_val = x;
                                        return WORD;
                                    }
                                }
                            
    "0x"[0-9a-fA-F]{1,2}        { yylval.byte_val = strtol(yytext, NULL, 16); return BYTE; }
    "0x"[0-9a-fA-F]{3,4}        { yylval.word_val = strtol(yytext, NULL, 16); return WORD; }


    "ld"                        { return OPC_LD; }

    ";"                         { BEGIN(COMMENT); }
    .                           ; 
}

<COMMENT>{
    [\n]                        {  ++line; BEGIN(INITIAL); }
    .                           ;
}

%%
