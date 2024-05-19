%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "sysy.tab.hpp"

using namespace std;

// line number
// int yylineno = 1;
// position in line
int yycolno = 1;

%}

/* 空白符和注释 */
WhiteSpace    [ \t]*
LineComment   "//".*
BlockComment   \/\*([^*]|(\*+[^*/]))*\*+\/
NewLine       \n|\r\n|\r

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

/* 运算符 */
UnaryOp        \!
AddOp          \+|\-
MulOp          \*|\/|\%
RelOp          <|>|<=|>=
EqOp           ==|\!=
LAndOp         &&
LOrOp          \|\|
%%

{WhiteSpace}    { yycolno += yyleng; }
{LineComment}   { yycolno += yyleng; }
{BlockComment}  { int i = 0; for(i = 0; i < strlen(yytext); i++) {if(yytext[i] == '\n' || yytext[i] == '\r') yylineno++; yycolno = 1;} }
{NewLine}       { yylineno++; yycolno = 1; }

"int"           { yycolno += yyleng; return INT; }
"void"          { yycolno += yyleng; return VOID; }
"return"        { yycolno += yyleng; return RETURN; }
"const"         { yycolno += yyleng; return CONST; }
"if"            { yycolno += yyleng; return IF; }
"else"          { yycolno += yyleng; return ELSE; }
"while"         { yycolno += yyleng; return WHILE; }
"break"         { yycolno += yyleng; return BREAK; }
"continue"      { yycolno += yyleng; return CONTINUE; }


{Identifier}    { yylval.str_val = new string(yytext); yycolno += yyleng; return IDENT; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); yycolno += yyleng; return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); yycolno += yyleng; return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); yycolno += yyleng; return INT_CONST; }

{UnaryOp}       { yylval.str_val = new string(yytext); yycolno += yyleng; return UNARYOP; }
{AddOp}         { yylval.str_val = new string(yytext); yycolno += yyleng; return ADDOP; }
{MulOp}         { yylval.str_val = new string(yytext); yycolno += yyleng; return MULOP; }
{RelOp}         { yylval.str_val = new string(yytext); yycolno += yyleng; return RELOP; }
{EqOp}          { yylval.str_val = new string(yytext); yycolno += yyleng; return EQOP; }
{LAndOp}        { yylval.str_val = new string(yytext); yycolno += yyleng; return LANDOP; }
{LOrOp}         { yylval.str_val = new string(yytext); yycolno += yyleng; return LOROP; }

.               { yycolno += yyleng; return yytext[0]; }

%%
