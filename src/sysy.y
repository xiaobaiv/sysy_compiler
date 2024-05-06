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
#include <string.h>
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
%token INT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT UNARYOP ADDOP MULOP RELOP EQOP LOROP LANDOP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt BlockItem BlockItemList FuncFParams FuncFParam FuncRPParams
%type <ast_val> Exp UnaryExp PrimaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp LVal ConstExp ExpList
%type <ast_val> Decl ConstDecl BType ConstDef ConstDefList ConstInitVal
%type <ast_val> VarDecl VarDef VarDefList InitVal
%type <ast_val> CompUnit
%type <int_val> Number

%glr-parser

%%
/* 正确，但是有些冗余，多使用了一个other_comp_unit  
CompUnit
  : OtherCompUnit {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->other_comp_unit = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

OtherCompUnit
  : FuncDef {
    auto comp_unit = new OtherCompUnitAST();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    comp_unit->option = OtherCompUnitAST::Option::COMPUNIT0;
    $$ = comp_unit;
    cout << "debug, FucnDef" << endl;
  }
  | OtherCompUnit FuncDef {
    auto comp_unit = new OtherCompUnitAST();
    comp_unit->func_def = unique_ptr<BaseAST>($2);
    comp_unit->other_comp_unit = unique_ptr<BaseAST>($1);
    comp_unit->option = OtherCompUnitAST::Option::COMPUNIT1;
    $$ = comp_unit;
    cout << "debug, CompUnit" << endl;
  }
  ;  */
/* class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  enum class Type { FUNCDEF, DECL } type;
  enum class Option { C0, C1 } option;
  std::unique_ptr<BaseAST> other_comp_unit;
  std::unique_ptr<BaseAST> func_def_or_decl;

  void Dump() const override {
    std::cout << "CompUnitAST { ";
    switch (type) {
      case Type::FUNCDEF:
        if(option == Option::C1) {
          other_comp_unit->Dump();
          std::cout << ", ";
        }
        func_def_or_decl->Dump();
        break;
      case Type::DECL:
        if(option == Option::C1) {
          other_comp_unit->Dump();
          std::cout << ", ";
        }
        func_def_or_decl->Dump();
        break;
    }
    std::cout << " }";
  }
}; */
// 优化后的 CompUnit, 使用ast作为暂时存储右侧的CompUnit的缓存
  CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->type = CompUnitAST::Type::FUNCDEF;
    comp_unit->option = CompUnitAST::Option::C0;
    comp_unit->func_def_or_decl = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  | CompUnit FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->type = CompUnitAST::Type::FUNCDEF;
    comp_unit->option = CompUnitAST::Option::C1;
    comp_unit->func_def_or_decl = unique_ptr<BaseAST>($2);
    comp_unit->other_comp_unit = move(ast);
    ast = move(comp_unit);
  }
  | CompUnit Decl {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->type = CompUnitAST::Type::DECL;
    comp_unit->option = CompUnitAST::Option::C1;
    comp_unit->func_def_or_decl = unique_ptr<BaseAST>($2);
    comp_unit->other_comp_unit = move(ast);
    ast = move(comp_unit);
  }
  | Decl {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->type = CompUnitAST::Type::DECL;
    comp_unit->option = CompUnitAST::Option::C0;
    comp_unit->func_def_or_decl = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;  

/* CompUnit 1. 设计unique_ptr 和 普通指针的转换，有问题。 2. 如果不设置 $$ ，会报错，因为调用$2
  : FuncDef {
    auto comp_unit = new CompUnitAST();
    comp_unit->func_def = std::unique_ptr<BaseAST>($1);
    comp_unit->type = CompUnitAST::Type::FUNCDEF;
    ast = comp_unit;
    cout << "debug, FucnDef" << endl;
    $$ = ast;
  }
  | CompUnit FuncDef {
    auto comp_unit = new CompUnitAST();
    comp_unit->other_comp_unit = std::unique_ptr<BaseAST>($1);
    comp_unit->func_def = std::unique_ptr<BaseAST>($2);
    comp_unit->type = CompUnitAST::Type::COMPUNIT;
    ast = comp_unit;
    cout << "debug, CompUnit" << endl;
    $$ = ast;
  }
  ; */

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->option = FuncDefAST::Option::F0;
    $$ = ast;
  }
  | FuncType IDENT '(' {
    global_stack.push(List());
  } FuncFParams ')' Block {

    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    auto _func_f_params = new FuncFParamsAST(global_stack.top());
    global_stack.pop();
    ast->func_fparams = unique_ptr<BaseAST>(_func_f_params);
    ast->block = unique_ptr<BaseAST>($7);
    ast->option = FuncDefAST::Option::F1;
    $$ = ast;
  }
  ;

FuncFParams
  : FuncFParam {
    auto func_f_param = unique_ptr<BaseAST>($1);
    global_stack.top().emplace_back(ListType::FUNCFPARAM, std::move(func_f_param));
  }
  | FuncFParams ',' FuncFParam {
    auto func_f_param = unique_ptr<BaseAST>($3);
    global_stack.top().emplace_back(ListType::FUNCFPARAM, std::move(func_f_param));
  }
  ;


FuncFParam
  : BType IDENT {
    auto btype = unique_ptr<BaseAST>($1);
    auto ident = *unique_ptr<string>($2);
    $$ = new FuncFParamAST(ident, btype);
  }
  ;

FuncRPParams
  : {
    global_stack.push(List());
  } ExpList {
    auto ast = new FuncRParamsAST(global_stack.top());
    global_stack.pop();
    $$ = ast;
  }
  ;

ExpList
  : ExpList ',' Exp {
    auto exp = unique_ptr<BaseAST>($3);
    global_stack.top().emplace_back(ListType::EXP, std::move(exp));
  }
  | Exp {
    auto exp = unique_ptr<BaseAST>($1);
    global_stack.top().emplace_back(ListType::EXP, std::move(exp));
  }
  ;
// 同上, 不再解释
BType
  : INT {
    $$ = new BTypeAST("int");
  }
  ;

FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = "int";
    $$ = ast;
  }
  | VOID {
    auto ast = new FuncTypeAST();
    ast->type = "void";
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
  | IF '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::IF;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->if_stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt ELSE Stmt {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::IFELSE;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->if_stmt = unique_ptr<BaseAST>($5);
    ast->else_stmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' Stmt {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::WHILE;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->if_stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::BREAK;
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Type::CONTINUE;
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
  | IDENT '(' ')' {
    auto ast = new UnaryExpAST();
    ast->type = UnaryExpAST::Type::IDENT;
    ast->option = UnaryExpAST::Option::F0;
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '(' FuncRPParams ')' {
    auto ast = new UnaryExpAST();
    ast->type = UnaryExpAST::Type::IDENT;
    ast->option = UnaryExpAST::Option::F1;
    ast->ident = *unique_ptr<string>($1);
    ast->func_r_params = unique_ptr<BaseAST>($3);
    $$ = ast;
    cout << "debug" << endl;
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
/* void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
} */

void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  
    extern int yylineno;    // defined and maintained in lex
    extern char *yytext;    // defined and maintained in lex
    int len=strlen(yytext);
    int i;
    char buf[512]={0};
    for (i=0;i<len;++i)
    {
        sprintf(buf,"%s%d ",buf,yytext[i]);
    }
    fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);

}
