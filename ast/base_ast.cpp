#include "base_ast.hh"
var_index_t BaseAST::global_var_index = 0;
var_index_t BaseAST::global_label_index = 0;
SymbolTable BaseAST::symbol_table = SymbolTable();
std::string BaseAST::ir = "";
