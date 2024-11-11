%{
#include <stdio.h>
#include <string.h>
#include "TeaplAst.h"
#include "y.tab.hpp"
extern int line, col;
int c;
int calc(const char *s, int len);
%}
%start COMMENT1 COMMENT2


%%
<INITIAL>"\t" { col+=4; }
<INITIAL>"\n" { ++line; col = 1; }
<INITIAL>" " { ++col; }

<INITIAL>"//" { col+=2; BEGIN COMMENT1; }
<COMMENT1>"\n" { ++line; col=1; BEGIN INITIAL; }

<INITIAL>"/*" { col+=2; BEGIN COMMENT2; }
<COMMENT2>"*/" { col+=2; BEGIN INITIAL; }

<COMMENT1>.* { col++; }
<COMMENT2>.* { col++; }
<COMMENT2>"\n" { ++line; col=1; }

<INITIAL>"+" { 
    yylval.pos = A_Pos(line, col); ++col; return ADD; 
}
<INITIAL>"*" { 
    yylval.pos = A_Pos(line, col); ++col; return MUL; 
}
<INITIAL>"-" { 
    yylval.pos = A_Pos(line, col); ++col; return SUB; 
}
<INITIAL>"/" { 
    yylval.pos = A_Pos(line, col); ++col; return DIV; 
}



<INITIAL>"==" { yylval.pos = A_Pos(line, col); col+=2; return CEQ; }
<INITIAL>"!=" { yylval.pos = A_Pos(line, col); col+=2; return NEQ; }
<INITIAL>">=" { yylval.pos = A_Pos(line, col); col+=2; return GTE; }
<INITIAL>"<=" { yylval.pos = A_Pos(line, col); col+=2; return LTE; }
<INITIAL>">" {yylval.pos = A_Pos(line, col); ++col; return GT;}
<INITIAL>"<" {yylval.pos = A_Pos(line, col); ++col; return LT;}
<INITIAL>"&&" { yylval.pos = A_Pos(line, col); col+=2; return AND; }
<INITIAL>"||" { yylval.pos = A_Pos(line, col); col+=2; return OR; }
<INITIAL>"!" { yylval.pos = A_Pos(line, col); ++col; return NOT; }
<INITIAL>"=" { yylval.pos = A_Pos(line, col); ++col; return EQ; }
<INITIAL>"," { yylval.pos = A_Pos(line, col); ++col; return COMMA; }
<INITIAL>":" { yylval.pos = A_Pos(line, col); ++col; return COLON; }
<INITIAL>"." { yylval.pos = A_Pos(line, col); ++col; return DOT; }
<INITIAL>"->" { yylval.pos = A_Pos(line, col); col+=2; return RARROW; }

<INITIAL>"(" { yylval.pos = A_Pos(line, col); ++col; return LPAR; }
<INITIAL>")" { yylval.pos = A_Pos(line, col); ++col; return RPAR; }
<INITIAL>"[" { yylval.pos = A_Pos(line, col); ++col; return LSQ; }
<INITIAL>"]" { yylval.pos = A_Pos(line, col); ++col; return RSQ; }
<INITIAL>"{" { yylval.pos = A_Pos(line, col); ++col; return LBRA; }
<INITIAL>"}" { yylval.pos = A_Pos(line, col); ++col; return RBRA; }

<INITIAL>"fn" { yylval.pos = A_Pos(line, col); col+=2; return FN; }
<INITIAL>"struct" { yylval.pos = A_Pos(line, col); col+=6; return STRUCT; }
<INITIAL>"let" { yylval.pos = A_Pos(line, col); col+=3; return LET; }
<INITIAL>"int" { yylval.pos = A_Pos(line, col); col+=3; return INT; }
<INITIAL>"if" { yylval.pos = A_Pos(line, col); col+=2; return IF; }
<INITIAL>"else" { yylval.pos = A_Pos(line, col); col+=4; return ELSE; }
<INITIAL>"while" { yylval.pos = A_Pos(line, col); col+=5; return WHILE; }
<INITIAL>";" { yylval.pos = A_Pos(line, col); ++col; return SEMICOLON; }
<INITIAL>"ret" { yylval.pos = A_Pos(line, col); col+=3; return RET; }
<INITIAL>"continue" { yylval.pos = A_Pos(line, col); col+=8; return CONTINUE; }
<INITIAL>"break" { yylval.pos = A_Pos(line, col); col+=5; return BREAK; }

<INITIAL>[a-z_A-Z][a-z_A-Z0-9]* { 
    yylval.tokenId = A_TokenId(A_Pos(line, col), strdup(yytext));
    col+=yyleng;
    return IDENTIFIER;
}
<INITIAL>[1-9][0-9]* {
    yylval.tokenNum = A_TokenNum(A_Pos(line, col), calc(yytext, yyleng));
    col+=yyleng;
    return UNUM;
}

<INITIAL>0 {
    yylval.tokenNum = A_TokenNum(A_Pos(line, col), 0);
    ++col;
    return UNUM;
}


. {
    printf("Illegal input \"%c\"\n", yytext[0]);
}

%%

// This function takes a string of digits and its length as input, and returns the integer value of the string.
int calc(const char *s, int len) {
    int ret = 0;
    for(int i = 0; i < len; i++)
        ret = ret * 10 + (s[i] - '0');
    return ret;
}