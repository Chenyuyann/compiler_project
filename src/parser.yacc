%{
#include <stdio.h>
#include "TeaplAst.h"

extern A_pos pos;
extern A_program root;

extern int yylex(void);
extern "C"{
extern void yyerror(const char *s); 
extern int  yywrap();
}

%}

// TODO:
// your parser

%union {
  A_pos pos;
  A_tokenId tokenId;
  A_tokenNum tokenNum;
  A_type type;
  A_program program;
  A_programElementList programElementList;
  A_programElement programElement;
  A_arithExpr arithExpr;
  A_exprUnit exprUnit;
  A_structDef structDef;
  A_varDeclStmt varDeclStmt;
  A_fnDeclStmt fnDeclStmt;
  A_fnDef fnDef;
  A_varDecl varDecl;
  A_varDef varDef;
  A_leftVal leftVal;
  A_rightVal rightVal;
  A_boolExpr boolExpr;
  A_arithBiOpExpr arithBiOpExpr;
  A_arithUExpr arithUExpr;
  A_fnCall fnCall;
  A_indexExpr indexExpr;
  A_arrayExpr arrayExpr;
  A_memberExpr memberExpr;
  A_boolUnit boolUnit;
  A_boolBiOpExpr boolBiOpExpr;
  A_boolUOpExpr boolUOpExpr;
  A_comExpr comExpr;
  A_assignStmt assignStmt;
  A_rightValList rightValList;
  A_varDefScalar varDefScalar;
  A_varDefArray varDefArray;
  A_varDeclScalar varDeclScalar;
  A_varDeclArray varDeclArray;
  A_varDeclList varDeclList;
  A_paramDecl paramDecl;
  A_fnDecl fnDecl;
  A_codeBlockStmt codeBlockStmt;
  A_ifStmt ifStmt;
  A_whileStmt whileStmt;
  A_callStmt callStmt;
  A_returnStmt returnStmt;
  A_codeBlockStmtList codeBlockStmtList;
}

//token：lex返回的东西
%token <pos> ADD
%token <pos> SUB
%token <pos> MUL
%token <pos> DIV

%token <pos> SEMICOLON // ;
%token <pos> CEQ // ==
%token <pos> NEQ // !=
%token <pos> GT // >
%token <pos> GTE // >=
%token <pos> LT // <
%token <pos> LTE // <=
%token <pos> AND // &&
%token <pos> OR // ||
%token <pos> NOT // !
%token <pos> EQ // =
%token <pos> COMMA // ,
%token <pos> COLON // :
%token <pos> DOT // .
%token <pos> RARROW // ->
%token <pos> LPAR // (
%token <pos> RPAR // )
%token <pos> LSQ // [
%token <pos> RSQ // ]
%token <pos> LBRA // {
%token <pos> RBRA // }
%token <pos> FN // fn
%token <pos> STRUCT // struct
%token <pos> LET // let
%token <pos> INT // int
%token <pos> IF // if
%token <pos> ELSE // else
%token <pos> WHILE // while
%token <pos> RET // return
%token <pos> CONTINUE // continue
%token <pos> BREAK // break
%token <tokenNum> UNUM
%token <tokenId> IDENTIFIER

// 优先级
%left SEMICOLON
%left COMMA
%left WHILE
%left IF
%left ELSE
%left IDENTIFIER
%left EQ
%left OR
%left AND
%left LT LTE GT GTE CEQ NEQ
%left ADD SUB
%left MUL DIV
%right NOT
%right LBRA
%left RBRA
%left DOT
%right LPAR
%left RPAR

//非终结符
%type <program> Program
%type <arithExpr> ArithExpr
%type <programElementList> ProgramElementList
%type <programElement> ProgramElement
%type <exprUnit> ExprUnit
%type <structDef> StructDef
%type <varDeclStmt> VarDeclStmt
%type <fnDeclStmt> FnDeclStmt
%type <fnDef> FnDef
%type <type> Type
%type <boolExpr> BoolExpr
%type <boolUnit> BoolUnit
%type <assignStmt> AssignStmt
%type <leftVal> LeftVal
%type <rightVal> RightVal
%type <fnCall> FnCall
%type <rightValList> RightValList
%type <varDecl> VarDecl
%type <varDef> VarDef
%type <varDeclList> VarDeclList
%type <paramDecl> ParamDecl
%type <fnDecl> FnDecl
%type <codeBlockStmtList> CodeBlockStmtList
%type <codeBlockStmtList> InnerCodeBlockStmtList
%type <codeBlockStmt> CodeBlockStmt
%type <ifStmt> IfStmt
%type <whileStmt> WhileStmt
%type <callStmt> CallStmt
%type <returnStmt> ReturnStmt
%type <codeBlockStmt> ContinueStmt
%type <codeBlockStmt> BreakStmt
%type <boolUnit> BoolUnit_
%type <tokenNum> NUM


%start Program

%%                   /* beginning of rules section */

Program: ProgramElementList 
{  
  root = A_Program($1);
  $$ = A_Program($1);
}
;

ProgramElementList: ProgramElement ProgramElementList
{
  $$ = A_ProgramElementList($1, $2);
}
|
{
  $$ = nullptr;
}
;

ProgramElement: VarDeclStmt
{
  $$ = A_ProgramVarDeclStmt($1->pos, $1);
}
| StructDef
{
  $$ = A_ProgramStructDef($1->pos, $1);
}
| FnDeclStmt
{
  $$ = A_ProgramFnDeclStmt($1->pos, $1);
}
| FnDef
{
  $$ = A_ProgramFnDef($1->pos, $1);
}
| SEMICOLON
{
  $$ = A_ProgramNullStmt($1);
}
;


ArithExpr: ArithExpr ADD ArithExpr
{
  $$ = A_ArithBiOp_Expr($1->pos, A_ArithBiOpExpr($1->pos, A_add, $1, $3));
}
| ArithExpr SUB ArithExpr
{
  $$ = A_ArithBiOp_Expr($1->pos, A_ArithBiOpExpr($1->pos, A_sub, $1, $3));
}
| ArithExpr MUL ArithExpr
{
  $$ = A_ArithBiOp_Expr($1->pos, A_ArithBiOpExpr($1->pos, A_mul, $1, $3));
}
| ArithExpr DIV ArithExpr
{
  $$ = A_ArithBiOp_Expr($1->pos, A_ArithBiOpExpr($1->pos, A_div, $1, $3));
}
| ExprUnit
{
  $$ = A_ExprUnit($1->pos, $1);
}
;

ExprUnit: NUM
{
  $$ = A_NumExprUnit($1->pos, $1->num);
}
| IDENTIFIER
{
  $$ = A_IdExprUnit($1->pos, $1->id);
}
| LPAR ArithExpr RPAR
{
  $$ = A_ArithExprUnit($1, $2);
}
| FnCall
{
  $$ = A_CallExprUnit($1->pos, $1);
}
| LeftVal LSQ NUM RSQ
{
  $$ = A_ArrayExprUnit($1->pos, A_ArrayExpr($1->pos, $1, A_NumIndexExpr($3->pos, $3->num)));
} 
| LeftVal LSQ IDENTIFIER RSQ
{
  $$ = A_ArrayExprUnit($1->pos, A_ArrayExpr($1->pos, $1, A_IdIndexExpr($3->pos, $3->id)));
}
| LeftVal DOT IDENTIFIER
{
  $$ = A_MemberExprUnit($1->pos, A_MemberExpr($1->pos, $1, $3->id));
}
;

NUM: UNUM
{
  $$ = A_TokenNum($1->pos, $1->num);
}
| SUB UNUM
{
  $$ = A_TokenNum($1, -($2->num));
}

BoolExpr: BoolExpr AND BoolExpr
{
  $$ = A_BoolBiOp_Expr($1->pos, A_BoolBiOpExpr($1->pos, A_and, $1, $3));
}
| BoolExpr OR BoolExpr
{
  $$ = A_BoolBiOp_Expr($1->pos, A_BoolBiOpExpr($1->pos, A_or, $1, $3));
}
| BoolUnit
{
  $$ = A_BoolExpr($1->pos, $1);
}
;

BoolUnit: LPAR ExprUnit GT ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_gt, $2, $4));
}
| LPAR ExprUnit LT ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_lt, $2, $4));
}
| LPAR ExprUnit GTE ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_ge, $2, $4));
}
| LPAR ExprUnit LTE ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_le, $2, $4));
}
| LPAR ExprUnit CEQ ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_eq, $2, $4));
}
| LPAR ExprUnit NEQ ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_ne, $2, $4));
}
| LPAR BoolExpr RPAR
{
  $$ = A_BoolExprUnit($1, $2);
}
| NOT BoolUnit
{
  $$ = A_BoolUOpExprUnit($1, A_BoolUOpExpr($1, A_not, $2));
}
;

AssignStmt: LeftVal EQ RightVal SEMICOLON
{
  $$ = A_AssignStmt($1->pos, $1, $3);
}
;

LeftVal: IDENTIFIER
{
  $$ = A_IdExprLVal($1->pos, $1->id);
}
| LeftVal LSQ IDENTIFIER RSQ
{
  $$ = A_ArrExprLVal($1->pos, A_ArrayExpr($1->pos, $1, A_IdIndexExpr($3->pos, $3->id)));
}
| LeftVal LSQ NUM RSQ
{
  $$ = A_ArrExprLVal($1->pos, A_ArrayExpr($1->pos, $1, A_NumIndexExpr($3->pos, $3->num)));
}
| LeftVal DOT IDENTIFIER
{
  $$ = A_MemberExprLVal($1->pos, A_MemberExpr($1->pos, $1, $3->id));
}
;

RightVal: ArithExpr
{
  $$ = A_ArithExprRVal($1->pos, $1);
}
| BoolExpr
{
  $$ = A_BoolExprRVal($1->pos, $1);
}
;

FnCall: IDENTIFIER LPAR RightValList RPAR
{
  $$ = A_FnCall($1->pos, $1->id, $3);
}
| IDENTIFIER LPAR RPAR
{
  $$ = A_FnCall($1->pos, $1->id, nullptr);
}
;

RightValList: RightVal COMMA RightValList
{
  $$ = A_RightValList($1, $3);
}
| RightVal
{
  $$ = A_RightValList($1, nullptr);
}
;

VarDeclStmt: LET VarDecl SEMICOLON
{
  $$ = A_VarDeclStmt($2->pos, $2);
}
| LET VarDef SEMICOLON
{
  $$ = A_VarDefStmt($2->pos, $2);
}
;

VarDecl: IDENTIFIER COLON Type
{
  $$ = A_VarDecl_Scalar($1->pos, A_VarDeclScalar($1->pos, $1->id, $3));
}
| IDENTIFIER LSQ NUM RSQ COLON Type
{
  $$ = A_VarDecl_Array($1->pos, A_VarDeclArray($1->pos, $1->id, $3->num, $6));
}
| IDENTIFIER
{
  $$ = A_VarDecl_Scalar($1->pos, A_VarDeclScalar($1->pos, $1->id, nullptr));
}
| IDENTIFIER LSQ NUM RSQ
{
  $$ = A_VarDecl_Array($1->pos, A_VarDeclArray($1->pos, $1->id, $3->num, nullptr));
}
;

VarDef: IDENTIFIER COLON Type EQ RightVal
{
  $$ = A_VarDef_Scalar($1->pos, A_VarDefScalar($1->pos, $1->id, $3, $5));
}
| IDENTIFIER EQ RightVal
{
  $$ = A_VarDef_Scalar($1->pos, A_VarDefScalar($1->pos, $1->id, nullptr, $3));
}
| IDENTIFIER LSQ NUM RSQ COLON Type EQ LBRA RightValList RBRA
{
  $$ = A_VarDef_Array($1->pos, A_VarDefArray($1->pos, $1->id, $3->num, $6, $9));
}
| IDENTIFIER LSQ NUM RSQ EQ LBRA RightVal RBRA
{
  $$ = A_VarDef_Array($1->pos, A_VarDefArray($1->pos, $1->id, $3->num, nullptr, A_RightValList($7, nullptr)));
}
;

Type: INT
{
  $$ = A_NativeType($1, A_intTypeKind);
}
| IDENTIFIER
{
  $$ = A_StructType($1->pos, $1->id);
}
;

VarDeclList: VarDecl COMMA VarDeclList
{
  $$ = A_VarDeclList($1, $3);
}
| VarDecl
{
  $$ = A_VarDeclList($1, nullptr);
}
;

StructDef: STRUCT IDENTIFIER LBRA VarDeclList RBRA
{
  $$ = A_StructDef($1, $2->id, $4);
}
;

FnDef: FnDecl CodeBlockStmtList
{
  $$ = A_FnDef($1->pos, $1, $2);
}
;

FnDeclStmt: FnDecl SEMICOLON
{
  $$ = A_FnDeclStmt($1->pos, $1);
}
;

FnDecl: FN IDENTIFIER LPAR ParamDecl RPAR
{
  $$ = A_FnDecl($1, $2->id, $4, nullptr);
}
| FN IDENTIFIER LPAR ParamDecl RPAR RARROW Type
{
  $$ = A_FnDecl($1, $2->id, $4, $7);
}
;

ParamDecl: VarDeclList
{
  $$ = A_ParamDecl($1);
}
|
{
  $$ = A_ParamDecl(nullptr);
}
;

CodeBlockStmtList: LBRA InnerCodeBlockStmtList RBRA
{
  $$ = $2; 
}
;
InnerCodeBlockStmtList: CodeBlockStmt InnerCodeBlockStmtList
{
  $$ = A_CodeBlockStmtList($1, $2);
}
|
{
  $$ = nullptr;
}
;

CodeBlockStmt: VarDeclStmt
{
  $$ = A_BlockVarDeclStmt($1->pos, $1);
}
| AssignStmt
{
  $$ = A_BlockAssignStmt($1->pos, $1);
}
| CallStmt
{
  $$ = A_BlockCallStmt($1->pos, $1);
}
| IfStmt
{
  $$ = A_BlockIfStmt($1->pos, $1);
}
| WhileStmt
{
  $$ = A_BlockWhileStmt($1->pos, $1);
}
| ReturnStmt
{
  $$ = A_BlockReturnStmt($1->pos, $1);
}
| ContinueStmt
{
  $$ = A_BlockContinueStmt($1->pos);
}
| BreakStmt
{
  $$ = A_BlockBreakStmt($1->pos);
}
| SEMICOLON
{
  $$ = A_BlockNullStmt($1);
}
;

CallStmt: FnCall SEMICOLON
{
  $$ = A_CallStmt($1->pos, $1);
}
;

IfStmt: IF BoolUnit_ CodeBlockStmtList
{
  $$ = A_IfStmt($1, $2, $3, nullptr);
}
| IF BoolUnit_ CodeBlockStmtList ELSE CodeBlockStmtList
{
  $$ = A_IfStmt($1, $2, $3, $5);
}
;

WhileStmt: WHILE BoolUnit_ CodeBlockStmtList
{
  $$ = A_WhileStmt($1, $2, $3);
}
;

ReturnStmt: RET RightVal SEMICOLON
{
  $$ = A_ReturnStmt($1, $2);
}
| RET SEMICOLON
{
  $$ = A_ReturnStmt($1, nullptr);
}
;

ContinueStmt: CONTINUE SEMICOLON
{
  $$ = A_BlockContinueStmt($1);
}
;

BreakStmt: BREAK SEMICOLON
{
  $$ = A_BlockBreakStmt($1);
}
;

BoolUnit_: LPAR ExprUnit GT ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_gt, $2, $4));
}
| LPAR ExprUnit GTE ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_ge, $2, $4));
}
| LPAR ExprUnit LT ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_lt, $2, $4));
}
| LPAR ExprUnit LTE ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_le, $2, $4));
}
| LPAR ExprUnit CEQ ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_eq, $2, $4));
}
| LPAR ExprUnit NEQ ExprUnit RPAR
{
  $$ = A_ComExprUnit($2->pos, A_ComExpr($2->pos, A_ne, $2, $4));
}
| LPAR BoolExpr RPAR
{
  $$ = A_BoolExprUnit($2->pos, $2);
}
| LPAR NOT BoolUnit RPAR
{
  $$ = A_BoolUOpExprUnit($2, A_BoolUOpExpr($2, A_not, $3));
}
;

%%

extern "C"{
void yyerror(const char * s)
{
  fprintf(stderr, "%s\n",s);
}
int yywrap()
{
  return(1);
}
}


