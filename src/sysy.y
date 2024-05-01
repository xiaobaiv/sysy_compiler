%code requires {
  #include <memory>
  #include <string>
  #include "/root/compiler/sysy-make-template/ast/ast.hh"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "/root/compiler/sysy-make-template/ast/ast.hh"
// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数，有的是ast指针

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT UNARYOP ADDOP MULOP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp UnaryExp PrimaryExp MulExp AddExp
%type <int_val> Number

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = "int";
    $$ = ast;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

Exp
  : AddExp {
    auto ast = new ExpAST();
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto primary_exp = unique_ptr<BaseAST>($1);
    $$ = new UnaryExpAST(primary_exp);
  }
  | UNARYOP UnaryExp {
    auto op = unique_ptr<string>($1);
    auto unary_exp = unique_ptr<BaseAST>($2);
    $$ = new UnaryExpAST(*op, unary_exp);
  }
  | ADDOP UnaryExp {
    auto op = unique_ptr<string>($1);
    auto unary_exp = unique_ptr<BaseAST>($2);
    $$ = new UnaryExpAST(*op, unary_exp);
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto exp = unique_ptr<BaseAST>($2);
    $$ = new PrimaryExpAST(exp);
  }
  | Number {
    $$ = new PrimaryExpAST($1);
  }
  ;

MulExp
  : UnaryExp {
    auto unary_exp = unique_ptr<BaseAST>($1);
    $$ = new MulExpAST(unary_exp);
  }
  | MulExp MULOP UnaryExp {
    auto mul_exp = unique_ptr<BaseAST>($1);
    auto op = unique_ptr<string>($2);
    auto unary_exp = unique_ptr<BaseAST>($3);
    $$ = new MulExpAST(mul_exp, *op, unary_exp);
  }
  ;

AddExp
  : MulExp {
    auto mul_exp = unique_ptr<BaseAST>($1);
    $$ = new AddExpAST(mul_exp);
  }
  | AddExp ADDOP MulExp {
    auto add_exp = unique_ptr<BaseAST>($1);
    auto op = unique_ptr<string>($2);
    auto mul_exp = unique_ptr<BaseAST>($3);
    $$ = new AddExpAST(add_exp, *op, mul_exp);
  }

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
