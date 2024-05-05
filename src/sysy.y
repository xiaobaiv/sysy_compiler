%code requires {
  #include <memory>
  #include <string>
  #include "/root/compiler/sysy-make-template/ast/ast.hh"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <stack>
#include "/root/compiler/sysy-make-template/ast/ast.hh"
// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

/* enum class ListType { CONSTDEF, DECL, STMT }; */
stack<List> global_stack;


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
%token INT RETURN CONST
%token <str_val> IDENT UNARYOP ADDOP MULOP RELOP EQOP LOROP LANDOP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt BlockItem BlockItemList LVal ConstExp 
%type <ast_val> Exp UnaryExp PrimaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> Decl ConstDecl BType ConstDef ConstDefList ConstInitVal
%type <ast_val> VarDecl VarDef VarDefList InitVal
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
/* 
Decl          ::= ConstDecl;
ConstDecl     ::= "const" BType ConstDefList  ";";
ConstDefList  ::= ConstDefList "," ConstDef | ConstDef ;
BType         ::= "int";
ConstDef      ::= IDENT "=" ConstInitVal;
ConstInitVal  ::= ConstExp;

Block         ::= "{" BlockItemList "}";
BlockItemList ::= BlockItemList BlockItem | ε;
BlockItem     ::= Decl | Stmt;

LVal          ::= IDENT;
PrimaryExp    ::= "(" Exp ")" | LVal | Number;

ConstExp      ::= Exp;
 */
Block
  : '{' {
    global_stack.push(List());
  } BlockItemList '}' {
    $$ = new BlockAST(global_stack.top());
    global_stack.pop();
  }
  | '{' '}' {
    $$ = new BlockAST();
  }
  ;

BlockItemList
  :
  BlockItemList BlockItem 
  | BlockItem
  ;

BlockItem
  : Decl {
    auto decl = unique_ptr<BaseAST>($1);
    auto ast = new BlockItemAST(decl, BlockItemAST::Type::DECL);
    $$ = ast;
    global_stack.top().emplace_back(ListType::BLOCKITEM, std::move(ast));
  }
  | Stmt {
    auto stmt = unique_ptr<BaseAST>($1);
    auto ast = new BlockItemAST(stmt, BlockItemAST::Type::STMT);
    $$ = ast;
    global_stack.top().emplace_back(ListType::BLOCKITEM, std::move(ast));
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::RETURN;
    ast->option = StmtAST::Option::EXP1;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::RETURN;
    ast->option = StmtAST::Option::EXP0;
    $$ = ast;
  }

  | LVal '=' Exp ';' {
    auto lval = unique_ptr<BaseAST>($1);
    auto exp = unique_ptr<BaseAST>($3);
    $$ = new StmtAST(lval, exp);
  }
  | Block {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::BLOCK;
    ast->block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::EXP;
    ast->option = StmtAST::Option::EXP1;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::EXP;
    ast->option = StmtAST::Option::EXP0;
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->lor_exp = unique_ptr<BaseAST>($1);
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
    auto ast = new PrimaryExpAST();
    ast->type = PrimaryExpAST::Type::EXP;
    ast->exp_or_lval = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | Number {
    $$ = new PrimaryExpAST($1);
  }
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->type = PrimaryExpAST::Type::LVAL;
    ast->exp_or_lval = unique_ptr<BaseAST>($1);
    $$ = ast;
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
  ;

RelExp
  : AddExp {
    auto add_exp = unique_ptr<BaseAST>($1);
    $$ = new RelExpAST(add_exp);
  }
  | RelExp RELOP AddExp {
    auto rel_exp = unique_ptr<BaseAST>($1);
    auto op = unique_ptr<string>($2);
    auto add_exp = unique_ptr<BaseAST>($3);
    $$ = new RelExpAST(rel_exp, *op, add_exp);
  }
  ;

EqExp
  : RelExp {
    auto rel_exp = unique_ptr<BaseAST>($1);
    $$ = new EqExpAST(rel_exp);
  }
  | EqExp EQOP RelExp {
    auto eq_exp = unique_ptr<BaseAST>($1);
    auto op = unique_ptr<string>($2);
    auto rel_exp = unique_ptr<BaseAST>($3);
    $$ = new EqExpAST(eq_exp, *op, rel_exp);
  }
  ;

LAndExp
  : EqExp {
    auto eq_exp = unique_ptr<BaseAST>($1);
    $$ = new LAndExpAST(eq_exp);
  }
  | LAndExp LANDOP EqExp {
    auto land_exp = unique_ptr<BaseAST>($1);
    auto op = unique_ptr<string>($2);
    auto eq_exp = unique_ptr<BaseAST>($3);
    $$ = new LAndExpAST(land_exp, *op, eq_exp);
  }
  ;

LOrExp
  : LAndExp {
    auto land_exp = unique_ptr<BaseAST>($1);
    $$ = new LOrExpAST(land_exp);
  }
  | LOrExp LOROP LAndExp {
    auto lor_exp = unique_ptr<BaseAST>($1);
    auto op = unique_ptr<string>($2);
    auto land_exp = unique_ptr<BaseAST>($3);
    $$ = new LOrExpAST(lor_exp, *op, land_exp);
  }
  ;

Decl
  : ConstDecl {
    auto const_decl = unique_ptr<BaseAST>($1);
    $$ = new DeclAST(const_decl, DeclAST::Type::CONST);
  }
  | VarDecl {
    auto var_decl = unique_ptr<BaseAST>($1);
    $$ = new DeclAST(var_decl, DeclAST::Type::VAR);
  }
  ;

VarDecl
  : BType {
    global_stack.push(List());
  } VarDefList ';' {
    auto btype = unique_ptr<BaseAST>($1);
    $$ = new VarDeclAST(global_stack.top(), btype);
    global_stack.pop();
  }
  ;
VarDefList
  : VarDefList ',' VarDef {
    auto var_def = unique_ptr<BaseAST>($3);
    global_stack.top().emplace_back(ListType::VARDEF, std::move(var_def));
  }
  | VarDef {
    auto var_def = unique_ptr<BaseAST>($1);
    global_stack.top().emplace_back(ListType::VARDEF, std::move(var_def));
  }
  ;

ConstDecl 
  : CONST BType {
    global_stack.push(List());
  } ConstDefList ';' {
    auto btype = unique_ptr<BaseAST>($2);
    $$ = new ConstDeclAST(global_stack.top(), btype);
    global_stack.pop();
  }
  ;

ConstDefList
  : ConstDefList ',' ConstDef {
    auto const_def = unique_ptr<BaseAST>($3);
    global_stack.top().emplace_back(ListType::CONSTDEF, std::move(const_def));
  }
  | ConstDef {
    auto const_def = unique_ptr<BaseAST>($1);
    global_stack.top().emplace_back(ListType::CONSTDEF, std::move(const_def));
  }

BType
  : INT {
    $$ = new BTypeAST("int");
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto ident = *unique_ptr<string>($1);
    auto const_init_val = unique_ptr<BaseAST>($3);
    $$ = new ConstDefAST(ident, const_init_val);
  }
  ;

VarDef
  : IDENT {
    auto ident = *unique_ptr<string>($1);
    $$ = new VarDefAST(ident);
  }
  | IDENT '=' InitVal {
    auto ident = *unique_ptr<string>($1);
    auto init_val = unique_ptr<BaseAST>($3);
    $$ = new VarDefAST(ident, init_val);
  }
  ;

InitVal
  : Exp {
    auto exp = unique_ptr<BaseAST>($1);
    $$ = new InitValAST(exp);
  }
  ;

ConstInitVal
  : ConstExp {
    auto const_exp = unique_ptr<BaseAST>($1);
    $$ = new ConstInitValAST(const_exp);
  }
  ;

LVal
  : IDENT {
    $$ = new LValAST(*unique_ptr<string>($1));
  }
  ;

ConstExp
  : Exp {
    auto exp = unique_ptr<BaseAST>($1);
    $$ = new ConstExpAST(exp);
  }
  ;
%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
