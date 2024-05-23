#include "base_ast.hh"

class FuncDefAST : public BaseAST { // FuncDef       ::= FuncType IDENT "(" [FuncFParams] ")" Block;
 public:
 enum class Option { F0, F1} option;
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> func_fparams;
  std::unique_ptr<BaseAST> block;

/* ir 语法
FunDef ::= "fun" SYMBOL "(" [FunParams] ")" [":" Type] "{" FunBody "}";
FunParams ::= SYMBOL ":" Type {"," SYMBOL ":" Type};
FunBody ::= {Block};
Block ::= SYMBOL ":" {Statement} EndStatement;
Statement ::= SymbolDef | Store | FunCall;
EndStatement ::= Branch | Jump | Return;
 */

  // 生成ir 的函数 toIR, 结果存在 字符串 ir 中，遵循ir 语法
  ret_value_t toIR(std::string& ir) override {
    if (option == Option::F0) { // FuncDef       ::= FuncType IDENT "(" ")" Block; 遵循ir语法，生成ir语言FunDef ::= "fun" SYMBOL "(" [FunParams] ")" [":" Type] "{" FunBody "}";
      ir += "fun @" + ident + "()";
      func_type->toIR(ir);
      ir += " {\n";
      block->toIR(ir);
      ir += "}\n";
    } else if (option == Option::F1) { // FuncDef       ::= FuncType IDENT "(" FuncFParams ")" Block; 遵循ir语法，生成ir语言FunDef ::= "fun" SYMBOL "(" FunParams ")" [":" Type] "{" FunBody "}";
      ir += "fun @" + ident + "(";
      func_fparams->toIR(ir);
      ir += ")";
      func_type->toIR(ir);
      ir += " {\n";
      block->toIR(ir);
      ir += "}\n";
    } else {
      std::cerr << "FuncDefAST::toIR: unknown option" << std::endl;
    } 
    return {0, RetType::VOID};
  }

  void Dump() const override {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    if(option == Option::F1) {
      func_fparams->Dump();
    }
    block->Dump();
    std::cout << " }";
  }

  void toDot(std::string& dot) const override { // FuncDef       ::= FuncType IDENT "(" [FuncFParams] ")" Block; 需要显示终结符，并单独设为一个field, 就像LValAST那样
    std::string node_id = getUniqueID();
    if (option == Option::F0) { // FuncDef       ::= FuncType IDENT "(" ")" Block;
      std::string node_def = node_id + "[label=\"<f0> FuncType | <f1> IDENT: " + ident + " | <f2> ( | <f3> ) | <f4> Block\"];\n";
      dot += node_def;
      func_type->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + func_type->getUniqueID() + "\";\n";
      block->toDot(dot);
      dot += "\"" + node_id + "\":f4 ->" + "\"" + block->getUniqueID() + "\";\n";
    } else if (option == Option::F1) { // FuncDef       ::= FuncType IDENT "(" FuncFParams ")" Block;
      std::string node_def = node_id + "[label=\"<f0> FuncType | <f1> IDENT: " + ident + " | <f2> ( | <f3> FuncFParams | <f4> ) | <f5> Block\"];\n";
      dot += node_def;
      func_type->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + func_type->getUniqueID() + "\";\n";
      func_fparams->toDot(dot);
      dot += "\"" + node_id + "\":f3 ->" + "\"" + func_fparams->getUniqueID() + "\";\n";
      block->toDot(dot);
      dot += "\"" + node_id + "\":f5 ->" + "\"" + block->getUniqueID() + "\";\n";
    } else {
      std::cerr << "FuncDefAST::toDot: unknown option" << std::endl;
    }
  }
};

class FuncTypeAST : public BaseAST { // FuncType      ::= "void" | "int";
public:
  std::string type;
 
  ret_value_t toIR(std::string& ir) override {
    if(type == "int") {
      ir += ": i32";
    } else if(type == "void") {
      ir += "";
    } else {
      std::cerr << "FuncTypeAST::toIR: unknown type" << std::endl;
    }
    return {0, RetType::VOID};
  }
  void Dump() const override {
    std::cout << "FuncTypeAST { ";
    std::cout << type <<" }";
  }

  void toDot(std::string& dot) const override {
    std::string node_id = getUniqueID();
    std::string node_label = "FuncType: " + type;  // 直接将类型名称加入标签
    std::string node_def = node_id + " [label=\"" + node_label + "\"];\n";
    dot += node_def;
  }
};

class FuncFParamsAST : public BaseAST { // FuncFParams   ::= FuncFParam {"," FuncFParam};
public:
  List fparams_list;
  FuncFParamsAST(List &_fparams_list) {
    for(auto &item : _fparams_list) {
      fparams_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
  }



  void Dump() const override {
    std::cout << "FuncFParamsAST { ";
    for(auto &item : fparams_list) {
      item.second->Dump();
    }
    std::cout << " }";
  }
  void toDot(std::string& dot) const override { // FuncFParams   ::= FuncFParam {"," FuncFParam};
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"<f0> FuncFParam";
    for(int i = 1; i < fparams_list.size(); i++) {
      node_def += " | <f" + std::to_string(i*2 - 1) + "> , | <f" + std::to_string(i*2) + "> FuncFParam";
    }
    node_def += "\"];\n";
    dot += node_def;

    for(int i = 0; i < fparams_list.size(); i++) {
      fparams_list[i].second->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(i*2) + " ->" + "\"" + fparams_list[i].second->getUniqueID() + "\";\n";
    }
  }
};

class FuncFParamAST : public BaseAST { // FuncFParam    ::= BType IDENT ["[" "]" {"[" ConstExp "]"}];
public:
  enum class Option { C0, C1 } option;
  std::string ident;
  List const_exp_list;
  std::unique_ptr<BaseAST> btype;
  FuncFParamAST(Option _option , const std::string &_ident, List &_const_exp_list, std::unique_ptr<BaseAST> &_btype) : option(_option), ident(_ident), btype(std::move(_btype)) {
    for(auto &item : _const_exp_list) {
      const_exp_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
  }
  void Dump() const override {
    std::cout << "FuncFParamAST { " << ident;
    for(auto &item : const_exp_list) {
      std::cout << "[" ;
      item.second->Dump();
      std::cout << "]";
    }
    std::cout << " }";
  }

  void toDot(std::string& dot) const override { // FuncFParam    ::= BType IDENT ["[" "]" {"[" ConstExp "]"}]; 需要显示终结符，并单独设为一个field, 就像LValAST那样
    std::string node_id = getUniqueID();
    if(option == Option::C0) { // FuncFParam    ::= BType IDENT ;
      std::string node_def = node_id + "[label=\"<f0> BType | <f1> IDENT: " + ident + "\"];\n";
      dot += node_def;
      btype->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + btype->getUniqueID() + "\";\n";
    } else if(option == Option::C1) { // FuncFParam    ::= BType IDENT "[" "]" {"[" ConstExp "]"};
      std::string node_def = node_id + "[label=\"<f0> BType | <f1> IDENT: " + ident + " | <f2> [ | <f3> ]";
      for(int i = 0; i < const_exp_list.size(); i++) {
        node_def += " | <f" + std::to_string(i*3 + 4) + "> [ | <f" + std::to_string(i*3 + 5) + "> ConstExp | <f" + std::to_string(i*3 + 6) + "> ]";
      }
      node_def += "\"];\n";
      dot += node_def;

      btype->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + btype->getUniqueID() + "\";\n";

      for(int i = 0; i < const_exp_list.size(); i++) {
        const_exp_list[i].second->toDot(dot);
        dot += "\"" + node_id + "\":f" + std::to_string(i*3 + 5) + " ->" + "\"" + const_exp_list[i].second->getUniqueID() + "\";\n";
      }

    } else {
      std::cerr << "FuncFParamAST::toDot: unknown option" << std::endl;
    }
  }

};

class BlockAST : public BaseAST { // Block         ::= "{" {BlockItem} "}";
public:
  List block_item_list;
  BlockAST() {}
  BlockAST(List &_block_item_list) {
    for(auto &item : _block_item_list) {
      block_item_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
  }

  ret_value_t toIR(std::string& ir) override { // %entry:
    ir += "%entry:\n";
    for(auto &item : block_item_list) {
      item.second->toIR(ir);
    }
    return {0, RetType::VOID};
  }


  void Dump() const override {
    std::cout << "BlockAST { ";
    for(auto &item : block_item_list) {
      item.second->Dump();
    }
    std::cout << " }";
  }

  void toDot(std::string& dot) const override { // Block         ::= "{" {BlockItem} "}";
    std::string node_id = getUniqueID();
    if(block_item_list.size() == 0) { // Block         ::= "{" "}";
      std::string node_def = node_id + "[label=\"<f0> \\{ |<f1> \\}" + "\"];\n";
      dot += node_def;
    } else { // Block         ::= "{" {BlockItem} "}";
      std::string node_def = node_id + "[label=\"<f0> \\{";
      for(int i = 0; i < block_item_list.size(); i++) {
        node_def += " | <f" + std::to_string(i + 1) + "> BlockItem";
      }
      node_def += " | <f" + std::to_string(block_item_list.size() + 1) + "> \\}" + "\"];\n";
      dot += node_def;

      for(int i = 0; i < block_item_list.size(); i++) {
        block_item_list[i].second->toDot(dot);
        dot += "\"" + node_id + "\":f" + std::to_string(i + 1) + " ->" + "\"" + block_item_list[i].second->getUniqueID() + "\";\n";
      }
    } 
  }

};

class BlockItemAST : public BaseAST { // BlockItem     ::= Decl | Stmt;
public:

  std::unique_ptr<BaseAST> decl_or_stmt;
  enum class Type { DECL, STMT } type;
  
  BlockItemAST(std::unique_ptr<BaseAST> &_decl_or_stmt, Type _type) : decl_or_stmt(std::move(_decl_or_stmt)), type(_type) {}
  
  ret_value_t toIR(std::string& ir) override {
    if(type == Type::DECL) { // BlockItem     ::= Decl;
      decl_or_stmt->toIR(ir);
    } else if(type == Type::STMT) { // BlockItem     ::= Stmt;
      decl_or_stmt->toIR(ir);
    } else {
      std::cerr << "BlockItemAST::toIR: unknown type" << std::endl;
    }
    return {0, RetType::VOID};
  }

  void Dump() const override {
    std::cout << "BlockItemAST { ";
    switch (type) {
      case Type::DECL:
        decl_or_stmt->Dump();
        break;
      case Type::STMT:
        decl_or_stmt->Dump();
        break;
    }
    std::cout << " }";
  }

  void toDot(std::string& dot) const override { 
    std::string node_id = getUniqueID();
    if(type == Type::DECL) { // BlockItem     ::= Decl;
      std::string node_def = node_id + "[label=\"<f0> Decl\"];\n";
      dot += node_def;
      decl_or_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + decl_or_stmt->getUniqueID() + "\";\n";
    } else if(type == Type::STMT) { // BlockItem     ::= Stmt;
      std::string node_def = node_id + "[label=\"<f0> Stmt\"];\n";
      dot += node_def;
      decl_or_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + decl_or_stmt->getUniqueID() + "\";\n";
    } else {
      std::cerr << "BlockItemAST::toDot: unknown type" << std::endl;
    }
  }
};

class StmtAST : public BaseAST { 
public:
  enum class Type { ASSIGN, RETURN, BLOCK, EXP ,IF, IFELSE, WHILE, BREAK, CONTINUE} type;
  enum class Option { EXP0, EXP1 } option;
  std::unique_ptr<BaseAST> lval;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> if_stmt;
  std::unique_ptr<BaseAST> else_stmt;
  StmtAST(std::unique_ptr<BaseAST> &_lval, std::unique_ptr<BaseAST> &_exp) : lval(std::move(_lval)), exp(std::move(_exp)) {
    type = Type::ASSIGN;
  }
  StmtAST() {}

  ret_value_t toIR(std::string& ir) override {
    if (type == Type::RETURN) {
      if(option == Option::EXP0) { // Stmt          ::= "return" ";";
        ir += "\tret\n";
      } else if (option == Option::EXP1) { // Stmt          ::= "return" Exp ";";
        ret_value_t ret = exp->toIR(ir);
        if(ret.second == RetType::NUMBER) {
          ir += "\tret " + std::to_string(ret.first.number) + "\n";
        } else if (ret.second == RetType::INDEX) {
          ir += "\tret %" + std::to_string(ret.first.number) + "\n";
        } else if (ret.second == RetType::VOID) {
          ir += "\tret\n";
        } else if (ret.second == RetType::IDENT) {
          ir += loadIR(ret);
          ir += "\tret %" + std::to_string(global_var_index - 1) + "\n";
        } else {
          std::cerr << "StmtAST::toIR: unknown ret type" << std::endl;
        }
      } else {
        std::cerr << "StmtAST::toIR: unknown option" << std::endl;
      }
    } else if(type == Type::ASSIGN) {
      ret_value_t exp_ret = exp->toIR(ir);
      ret_value_t lval_ret = lval->toIR(ir);
      ir += storeIR(exp_ret, lval_ret);
    }
    return {0, RetType::VOID};
  }

  void Dump() const override {
    std::cout << "StmtAST { ";
    switch (type) {
      case Type::ASSIGN:
        lval->Dump();
        std::cout << " = ";
        exp->Dump();
        break;
      case Type::RETURN:
        std::cout << "return ";
        if(option == Option::EXP1) {
          exp->Dump();
        }
        break;
      case Type::BLOCK:
        block->Dump();
        break;
      case Type::EXP:
        if(option == Option::EXP1) {
          exp->Dump();
        }
        break;
      case Type::IF:
        std::cout << "if ";
        exp->Dump();
        std::cout << " then ";
        if_stmt->Dump();
        break;
      case Type::IFELSE:
        std::cout << "if ";
        exp->Dump();
        std::cout << " then ";
        if_stmt->Dump();
        std::cout << " else ";
        else_stmt->Dump();
        break;
      case Type::WHILE:
        std::cout << "while ";
        exp->Dump();
        std::cout << " do ";
        if_stmt->Dump();
        break;
      case Type::BREAK:
        std::cout << "break";
        break;
      case Type::CONTINUE:
        std::cout << "continue";
        break;
      default:
        break;
    }
    std::cout << " }";
  }

  void toDot(std::string& dot) const override {
    /* Stmt          ::= LVal "=" Exp ";"
                | [Exp] ";"
                | Block
                | "if" "(" Exp ")" Stmt ["else" Stmt]
                | "while" "(" Exp ")" Stmt
                | "break" ";"
                | "continue" ";"
                | "return" [Exp] ";"; */
    std::string node_id = getUniqueID();
    if (type == Type::ASSIGN) { // Stmt          ::= LVal "=" Exp ";"
      std::string node_def = node_id + "[label=\"<f0> LVal | <f1> = | <f2> Exp\"];\n";
      dot += node_def;
      lval->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + lval->getUniqueID() + "\";\n";
      exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + exp->getUniqueID() + "\";\n";
    } else if (type == Type::RETURN) { // Stmt          ::= "return" [Exp] ";";
      if(option == Option::EXP0) { // Stmt          ::= "return" ";";
        std::string node_def = node_id + "[label=\"<f0> return | <f1> ;\"];\n";
        dot += node_def;
      } else if(option == Option::EXP1) { // Stmt          ::= "return" Exp ";";
        std::string node_def = node_id + "[label=\"<f0> return | <f1> Exp | <f2> ;\"];\n";
        dot += node_def;
        exp->toDot(dot);
        dot += "\"" + node_id + "\":f1 ->" + "\"" + exp->getUniqueID() + "\";\n";
      } else {
        std::cerr << "StmtAST::toDot: unknown option" << std::endl;
      }
    } else if (type == Type::BLOCK) { // Stmt          ::= Block;
      std::string node_def = node_id + "[label=\"<f0> Block\"];\n";
      dot += node_def;
      block->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + block->getUniqueID() + "\";\n";
    } else if (type == Type::EXP) { // Stmt          ::= [Exp] ";";
      if(option == Option::EXP0) { // Stmt          ::= ";";
        std::string node_def = node_id + "[label=\"<f0> ;\"];\n";
        dot += node_def;
      } else if(option == Option::EXP1) { // Stmt          ::= Exp ";";
        std::string node_def = node_id + "[label=\"<f0> Exp | <f1> ;\"];\n";
        dot += node_def;
        exp->toDot(dot);
        dot += "\"" + node_id + "\":f0 ->" + "\"" + exp->getUniqueID() + "\";\n";
      } else {
        std::cerr << "StmtAST::toDot: unknown option" << std::endl;
      }
    } else if (type == Type::IF) { // Stmt          ::= "if" "(" Exp ")" Stmt;
      std::string node_def = node_id + "[label=\"<f0> if | <f1> ( | <f2> Exp | <f3> ) | <f4> Stmt\"];\n";
      dot += node_def;
      exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + exp->getUniqueID() + "\";\n";
      if_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f4 ->" + "\"" + if_stmt->getUniqueID() + "\";\n";
    } else if (type == Type::IFELSE) { // Stmt          ::= "if" "(" Exp ")" Stmt "else" Stmt;
      std::string node_def = node_id + "[label=\"<f0> if | <f1> ( | <f2> Exp | <f3> ) | <f4> Stmt | <f5> else | <f6> Stmt\"];\n";
      dot += node_def;
      exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + exp->getUniqueID() + "\";\n";
      if_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f4 ->" + "\"" + if_stmt->getUniqueID() + "\";\n";
      else_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f6 ->" + "\"" + else_stmt->getUniqueID() + "\";\n";
    } else if (type == Type::WHILE) { // Stmt          ::= "while" "(" Exp ")" Stmt;
      std::string node_def = node_id + "[label=\"<f0> while | <f1> ( | <f2> Exp | <f3> ) | <f4> Stmt\"];\n";
      dot += node_def;
      exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + exp->getUniqueID() + "\";\n";
      if_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f4 ->" + "\"" + if_stmt->getUniqueID() + "\";\n";
    } else if (type == Type::BREAK) { // Stmt          ::= "break" ";";
      std::string node_def = node_id + "[label=\"<f0> break | <f1> ;\"];\n";
      dot += node_def;
    } else if (type == Type::CONTINUE) { // Stmt          ::= "continue" ";";
      std::string node_def = node_id + "[label=\"<f0> continue | <f1> ;\"];\n";
      dot += node_def;
    } else {
      std::cerr << "StmtAST::toDot: unknown type" << std::endl;
    }
  }
};
