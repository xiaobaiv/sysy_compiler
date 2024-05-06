#include "base_ast.hh"

/* class OtherCompUnitAST : public BaseAST {
 public:
  enum class Option { COMPUNIT0, COMPUNIT1 } option;
  std::unique_ptr<BaseAST> other_comp_unit;
  std::unique_ptr<BaseAST> func_def;
  OtherCompUnitAST() {}
  void Dump() const override {
    std::cout << "OtherCompUnitAST { ";
    switch (option) {
      case Option::COMPUNIT0:
        func_def->Dump();
        break;
      case Option::COMPUNIT1:
        other_comp_unit->Dump();
        std::cout << ", ";
        func_def->Dump();
        break;
    }
    std::cout << " }";
  }
}; */

class DeclAST : public BaseAST {
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

class VarDeclAST : public BaseAST {
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
};

class BTypeAST : public BaseAST {
public:
    std::string btype;
    BTypeAST(const std::string &_btype) : btype(_btype) {}
    void Dump() const override {
        std::cout << "BType { " << btype << " }";
    }

};

class ConstDefAST : public BaseAST {
public:
    enum class Option { C0, C1 } option;
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;
    std::unique_ptr<BaseAST> const_exp;
    // 构造函数，option为C0, 无const_exp
    ConstDefAST(const std::string &_ident, std::unique_ptr<BaseAST> &_const_init_val) : ident(_ident), const_init_val(std::move(_const_init_val)) {
        option = Option::C0;
    }
    // 构造函数，option为C1, 有const_exp
    ConstDefAST(const std::string &_ident, std::unique_ptr<BaseAST> &_const_init_val, std::unique_ptr<BaseAST> &_const_exp) : ident(_ident), const_init_val(std::move(_const_init_val)), const_exp(std::move(_const_exp)) {
        option = Option::C1;
    }
    
    void Dump() const override {
        std::cout << "ConstDefAST { " << ident ;
        if(option == Option::C1) {
            std::cout << ", [";
            const_exp->Dump();
            std::cout << "]";
        }
        std::cout << " = ";
        const_init_val->Dump();
        
        std::cout << " }";
    }
};

class VarDefAST : public BaseAST {
public:
  enum class Type { IDENT, IDENT_ASSIGN_INITVAL } type;
  enum class Option { C0, C1 } option;
  std::string ident;
  std::unique_ptr<BaseAST> init_val;
  std::unique_ptr<BaseAST> const_exp;
  
 // VarDefAST构造函数，IDENT_ASSIGN_INITVAL, C0
  VarDefAST(const std::string &_ident, std::unique_ptr<BaseAST> &_init_val, Option _option) : ident(_ident), init_val(std::move(_init_val)) {
    type = Type::IDENT_ASSIGN_INITVAL;
    option = Option::C0;
  }
  // VarDefAST构造函数，IDENT_ASSIGN_INITVAL, C1
  VarDefAST(const std::string &_ident, std::unique_ptr<BaseAST> &_init_val, std::unique_ptr<BaseAST> &_const_exp) : ident(_ident), init_val(std::move(_init_val)), const_exp(std::move(_const_exp)) {
    type = Type::IDENT_ASSIGN_INITVAL;
    option = Option::C1;
  }
  // VarDefAST构造函数，IDENT, C0
  VarDefAST(const std::string &_ident) : ident(_ident) {
    type = Type::IDENT;
    option = Option::C0;
  }
  // VarDefAST构造函数，IDENT, C1
  VarDefAST(const std::string &_ident, std::unique_ptr<BaseAST> &_const_exp) : ident(_ident), const_exp(std::move(_const_exp)) {
    type = Type::IDENT;
    option = Option::C1;
  }

  void Dump() const override {
    std::cout << "VarDefAST { ";
    switch (type) {
      case Type::IDENT:
        std::cout << ident;
        if(option == Option::C1) {
          std::cout << "[";
          const_exp->Dump();
          std::cout << "]";
        }
        break;
      case Type::IDENT_ASSIGN_INITVAL:
        std::cout << ident;
        if(option == Option::C1) {
          std::cout << "[";
          const_exp->Dump();
          std::cout << "]";
        }
        init_val->Dump();
        break;
    }
    std::cout << " }";
  }
};
class ConstInitValAST : public BaseAST {
public:
    enum class Type { CONSTEXP, ARRAY } type;
    enum class Option { C0, C1 } option;
    std::unique_ptr<BaseAST> const_exp;
    List const_exp_list;
    ConstInitValAST() {}
    ConstInitValAST(std::unique_ptr<BaseAST> &_const_exp) : const_exp(std::move(_const_exp)) { type = Type::CONSTEXP; }
    ConstInitValAST(List &_const_exp_list) {
        for(auto &item : _const_exp_list) {
            const_exp_list.push_back(std::make_pair(item.first, std::move(item.second)));
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
                for(auto &item : const_exp_list) {
                    item.second->Dump();
                }
                break;
            default:
                break;
        }
        std::cout << " }";
    }

};

class InitValAST : public BaseAST {
public:
    enum class Type { EXP, ARRAY } type;
    std::unique_ptr<BaseAST> exp;
    List exp_list;

    InitValAST() {}
    InitValAST(std::unique_ptr<BaseAST> &_exp) : exp(std::move(_exp)) { type = Type::EXP; }
    InitValAST(List &_exp_list) { 
        for(auto &item : _exp_list) {
            exp_list.push_back(std::make_pair(item.first, std::move(item.second)));
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
        for(auto &item : exp_list) {
          item.second->Dump();
        }
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
  std::string ident;
  std::unique_ptr<BaseAST> btype;
  FuncFParamAST(const std::string &_ident, std::unique_ptr<BaseAST> &_btype) : ident(_ident), btype(std::move(_btype)) {}
  void Dump() const override {
    std::cout << "FuncFParamAST { " << ident << ", ";
    btype->Dump();
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


