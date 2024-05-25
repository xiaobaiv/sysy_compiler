#include "base_ast.hh"
var_index_t BaseAST::global_var_index = 0;
var_index_t BaseAST::global_label_index = 0;
SymbolTable BaseAST::symbol_table = SymbolTable();
std::string BaseAST::ir = "decl @getint(): i32\ndecl @getch(): i32\ndecl @getarray(*i32): i32\ndecl @putint(i32)\ndecl @putch(i32)\ndecl @putarray(i32, *i32)\ndecl @starttime()\ndecl @stoptime()\n\n";
std::stack<var_index_t> BaseAST::loop_stack = std::stack<var_index_t>();
