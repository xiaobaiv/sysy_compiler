#include "base_ast.hh"

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
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

 /*  void toDot(std::string& dot) const override {
    std::string node_id = getUniqueID();
    if(type == Type::FUNCDEF) {
      if(option == Option::C0) { // CompUnit      ::=  FuncDef;
        std::string node_def = node_id + "[label=\"<f0> FuncDef\"];\n";
        dot += node_def;
        func_def_or_decl->toDot(dot);
        dot += "\"" + node_id + "\":f0 ->" + "\"" + func_def_or_decl->getUniqueID() + "\";\n";
      } else if (option == Option::C1) { // CompUnit      ::=  CompUnit FuncDef; 按照顺序展开
        std::string node_def = node_id + "[label=\"<f0> CompUnit | <f1> FuncDef\"];\n";
        dot += node_def;

        other_comp_unit->toDot(dot);
        dot += "\"" + node_id + "\":f0 ->" + "\"" + other_comp_unit->getUniqueID() + "\";\n";

        func_def_or_decl->toDot(dot);
        dot += "\"" + node_id + "\":f1 ->" + "\"" + func_def_or_decl->getUniqueID() + "\";\n";
      } else {
        std::cerr << "CompUnitAST::toDot: unknown option" << std::endl;
      }
    } else if(type == Type::DECL) {
      if(option == Option::C0) { // CompUnit      ::=  Decl;
        std::string node_def = node_id + "[label=\"<f0> Decl\"];\n";
        dot += node_def;
        func_def_or_decl->toDot(dot);
        dot += "\"" + node_id + "\":f0 ->" + "\"" + func_def_or_decl->getUniqueID() + "\";\n";
      } else if (option == Option::C1) { // CompUnit      ::=  CompUnit Decl; 按照顺序展开
        std::string node_def = node_id + "[label=\"<f0> CompUnit | <f1> Decl\"];\n";
        dot += node_def;

        other_comp_unit->toDot(dot);
        dot += "\"" + node_id + "\":f0 ->" + "\"" + other_comp_unit->getUniqueID() + "\";\n";

        func_def_or_decl->toDot(dot);
        dot += "\"" + node_id + "\":f1 ->" + "\"" + func_def_or_decl->getUniqueID() + "\";\n";
      } else {
        std::cerr << "CompUnitAST::toDot: unknown option" << std::endl;
      }
    } else {
      std::cerr << "CompUnitAST::toDot: unknown type" << std::endl;
    }
  } */
    

}; 

class DeclAST : public BaseAST { // Decl          ::= ConstDecl | VarDecl;
 public:

    std::unique_ptr<BaseAST> const_or_var_decl;
    enum class Type { CONST, VAR } type;

    DeclAST(std::unique_ptr<BaseAST> &_const_or_var_decl, Type _type) : const_or_var_decl(std::move(_const_or_var_decl)), type(_type) {}

    void Dump() const override {
        std::cout << "DeclAST { ";
        switch (type) {
            case Type::CONST:
                const_or_var_decl->Dump();
                break;
            case Type::VAR:
                const_or_var_decl->Dump();
                break;
        }
        std::cout << " }";
    }

    /* void toDot(std::string& dot) const override {
        std::string node_id = getUniqueID();
        if(type == Type::CONST) { // Decl          ::= ConstDecl;
            std::string node_def = node_id + "[label=\"<f0> ConstDecl\"];\n";
            dot += node_def;
            const_or_var_decl->toDot(dot);
            dot += "\"" + node_id + "\":f0 ->" + "\"" + const_or_var_decl->getUniqueID() + "\";\n";
        } else if(type == Type::VAR) { // Decl          ::= VarDecl;
            std::string node_def = node_id + "[label=\"<f0> VarDecl\"];\n";
            dot += node_def;
            const_or_var_decl->toDot(dot);
            dot += "\"" + node_id + "\":f0 ->" + "\"" + const_or_var_decl->getUniqueID() + "\";\n";
        } else {
            std::cerr << "DeclAST::toDot: unknown type" << std::endl;
        }
    } */

};

// TODO: ConstDeclAST
class ConstDeclAST : public BaseAST {
public:
  List const_def_list;
  std::unique_ptr<BaseAST> btype;

  ConstDeclAST(List &_const_def_list, std::unique_ptr<BaseAST> &_btype) {
    for(auto &item : _const_def_list) {
      const_def_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
    btype = std::move(_btype);
  }
  void Dump() const override {
    std::cout << "ConstDeclAST { ";
    btype->Dump();
    for(auto &item : const_def_list) {
      std::cout << ", ";
      item.second->Dump();
    }
    std::cout << " }";
  }
};

class VarDeclAST : public BaseAST { // VarDecl       ::= BType VarDef {"," VarDef} ";";
public:
  List var_def_list;
  std::unique_ptr<BaseAST> btype;

  VarDeclAST(List &_var_def_list, std::unique_ptr<BaseAST> &_btype) {
    for(auto &item : _var_def_list) {
      var_def_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
    btype = std::move(_btype);
  }
  void Dump() const override {
    std::cout << "VarDeclAST { ";
    btype->Dump();
    for(auto &item : var_def_list) {
      std::cout << ", ";
      item.second->Dump();
    }
    std::cout << " }";
  }

  /* void toDot(std::string& dot) const override { // VarDecl       ::= BType VarDef {"," VarDef} ";";
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"<f0> BType | <f1> VarDef";
    for(int i = 0; i < var_def_list.size(); i++) {
      node_def += " | <f" + std::to_string(i + 2) + "> VarDef";
    }
    node_def += "\"];\n";
    dot += node_def;

    btype->toDot(dot);
    dot += "\"" + node_id + "\":f0 ->" + "\"" + btype->getUniqueID() + "\";\n";

    for(int i = 0; i < var_def_list.size(); i++) {
      var_def_list[i].second->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(i + 1) + " ->" + "\"" + var_def_list[i].second->getUniqueID() + "\";\n";
    }
    
  } */

};

class BTypeAST : public BaseAST { // BType         ::= "int";
public:
    std::string btype;
    BTypeAST(const std::string &_btype) : btype(_btype) {}
    void Dump() const override {
        std::cout << "BType { " << btype << " }";
    }

    /* void toDot(std::string& dot) const override {
      // 生成当前节点的唯一标识符
      std::string node_id = getUniqueID();
      std::string node_label = "BType: " + btype;  // 直接将类型名称加入标签
      std::string node_def = node_id + " [label=\"" + node_label + "\"];\n";
      dot += node_def;
    } */


};

class ConstDefAST : public BaseAST {
public:
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;
    List const_exp_list;
    
    // ConstDefAST构造函数，ConstDef      ::= IDENT {"[" ConstExp "]"} "=" ConstInitVal;, 使用const_exp_list
    ConstDefAST(const std::string &_ident, List &_const_exp_list, std::unique_ptr<BaseAST> &_const_init_val) : ident(_ident), const_init_val(std::move(_const_init_val)) {
        for(auto &item : _const_exp_list) {
            const_exp_list.push_back(std::make_pair(item.first, std::move(item.second)));
        }
    }

    void Dump() const override {
        std::cout << "ConstDefAST { " << ident;
        for(auto &item : const_exp_list) {
            std::cout << "[" ;
            item.second->Dump();
            std::cout << "]";
        }
        std::cout << " = ";
        const_init_val->Dump();
        std::cout << " }";
    }
    
};

class VarDefAST : public BaseAST { // VarDef        ::= IDENT {"[" ConstExp "]"} | IDENT {"[" ConstExp "]"} "=" InitVal;
public:
  enum class Type { IDENT, INIT} type;
  std::string ident;
  List const_exp_list;
  std::unique_ptr<BaseAST> init_val;

  VarDefAST(const std::string &_ident, List &_const_exp_list) : ident(_ident) {
    for(auto &item : _const_exp_list) {
      const_exp_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
    type = Type::IDENT;
  }
  VarDefAST(const std::string &_ident, List &_const_exp_list, std::unique_ptr<BaseAST> &_init_val) : ident(_ident), init_val(std::move(_init_val)) {
    for(auto &item : _const_exp_list) {
      const_exp_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
    type = Type::INIT;
  }

  void Dump() const override {
    std::cout << "VarDefAST { " << ident;
    if(const_exp_list.size() > 0) {
      for(auto &item : const_exp_list) {
        std::cout << "[" ;
        item.second->Dump();
        std::cout << "]";
      }
    }
    if(type == Type::INIT) {
      std::cout << " = ";
    init_val->Dump();
    }
    std::cout << " }";
  }

};
class ConstInitValAST : public BaseAST { // ConstInitVal  ::= ConstExp | "{" [ConstInitVal {"," ConstInitVal}] "}";
public:
    enum class Type { CONSTEXP, ARRAY } type;
    std::unique_ptr<BaseAST> const_exp;
    List const_init_val_list;

    ConstInitValAST(std::unique_ptr<BaseAST> &_const_exp) : const_exp(std::move(_const_exp)) { type = Type::CONSTEXP; }
    ConstInitValAST(List &_const_init_val_list) {
        for(auto &item : _const_init_val_list) {
            const_init_val_list.push_back(std::make_pair(item.first, std::move(item.second)));
        }
        type = Type::ARRAY;
    }

    void Dump() const override {
        std::cout << "ConstInitValAST { ";
        switch (type) {
            case Type::CONSTEXP:
                const_exp->Dump();
                break;
            case Type::ARRAY:
                for(auto &item : const_init_val_list) {
                    item.second->Dump();
                }
                break;
            default:
                break;
        }
        std::cout << " }";
    }

};

class InitValAST : public BaseAST { // ConstInitVal  ::= ConstExp | "{" [ConstInitVal {"," ConstInitVal}] "}";
public:
    enum class Type { EXP, ARRAY } type;
    std::unique_ptr<BaseAST> exp;
    List init_val_list;

    InitValAST(std::unique_ptr<BaseAST> &_exp) : exp(std::move(_exp)) { type = Type::EXP; }
    InitValAST(List &_init_val_list) {
        for(auto &item : _init_val_list) {
            init_val_list.push_back(std::make_pair(item.first, std::move(item.second)));
        }
        type = Type::ARRAY;
    }

    void Dump() const override {
        std::cout << "InitValAST { ";
        switch (type) {
            case Type::EXP:
                exp->Dump();
                break;
            case Type::ARRAY:
                for(auto &item : init_val_list) {
                    item.second->Dump();
                }
                break;
            default:
                break;
        }
        std::cout << " }";
    }

};

class ConstExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;
    ConstExpAST(std::unique_ptr<BaseAST> &_exp) : exp(std::move(_exp)) {}
    void Dump() const override {
        std::cout << "ConstExpAST { ";
        exp->Dump();
        std::cout << " }";
    }
};


// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
 enum class Option { F0, F1} option;
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> func_fparams;
  std::unique_ptr<BaseAST> block;

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
};

// FuncType 是 BaseAST
class FuncTypeAST : public BaseAST {
public:
  std::string type;
 
  void Dump() const override {
    std::cout << "FuncTypeAST { ";
    std::cout << type <<" }";
  }
};

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
// Block 继承 BaseAST
// TODO: BlockAST
class BlockAST : public BaseAST {
public:
  List block_item_list;
  BlockAST() {}
  BlockAST(List &_block_item_list) {
    for(auto &item : _block_item_list) {
      block_item_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
  }
  void Dump() const override {
    std::cout << "BlockAST { ";
    for(auto &item : block_item_list) {
      item.second->Dump();
    }
    std::cout << " }";
  }

};

// Stmt 
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
};


// BlockItem 继承 BaseAST
class BlockItemAST : public BaseAST {
public:

  std::unique_ptr<BaseAST> decl_or_stmt;
  enum class Type { DECL, STMT } type;
  
  BlockItemAST(std::unique_ptr<BaseAST> &_decl_or_stmt, Type _type) : decl_or_stmt(std::move(_decl_or_stmt)), type(_type) {}
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
};

class FuncFParamsAST : public BaseAST {
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
};

class FuncFParamAST : public BaseAST {
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
};

class FuncRParamsAST : public BaseAST {
public:
  List exp_list;
  FuncRParamsAST(List &_exp_list) {
    for(auto &item : _exp_list) {
      exp_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
  }
  void Dump() const override {
    std::cout << "FuncRParamsAST { ";
    for(auto &item : exp_list) {
      item.second->Dump();
    }
    std::cout << " }";
  }
};


