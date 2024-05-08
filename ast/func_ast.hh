#include "base_ast.hh"

/* 还差6个toDot() */

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

    void toDot(std::string& dot) const override  {
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
  }
    

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

     void toDot(std::string& dot) const override  {
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
    } 

};


// TODO: ConstDeclAST
class ConstDeclAST : public BaseAST { // ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
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

  void toDot(std::string& dot) const override { // ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";"; 需要显示终结符，并单独设为一个field, 就像LValAST那样
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"<f0> const | <f1> BType";
    for(int i = 0; i < const_def_list.size(); i++) {
      if(i == 0) {
        node_def += " | <f" + std::to_string(i*2 + 2) + "> ConstDef";
      } else {
        node_def += " | <f" + std::to_string(i*2 - 1) + "> , | <f" + std::to_string(i*2) + "> ConstDef";
      }
    }
    node_def += " | <f" + std::to_string(const_def_list.size()*2 + 1) + "> ;\"];\n";
    dot += node_def;

    btype->toDot(dot);
    dot += "\"" + node_id + "\":f1 ->" + "\"" + btype->getUniqueID() + "\";\n";

    for(int i = 0; i < const_def_list.size(); i++) {
      const_def_list[i].second->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(i*2 + 2) + " ->" + "\"" + const_def_list[i].second->getUniqueID() + "\";\n";
    }
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

    void toDot(std::string& dot) const override { // VarDecl       ::= BType VarDef {"," VarDef} ";"; 需要显示终结符，并单独设为一个field, 就像上文那样
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"<f0> BType";
    for(int i = 0; i < var_def_list.size(); i++) {
      if(i == 0) {
        node_def += " | <f" + std::to_string(i*2 + 1) + "> VarDef";
      } else {
        node_def += " | <f" + std::to_string(i*2 + 1) + "> , | <f" + std::to_string(i*2 + 2) + "> VarDef";
      }
    }
    node_def += " | <f" + std::to_string(var_def_list.size()*2 + 1) + "> ;\"];\n";
    dot += node_def;

    btype->toDot(dot);
    dot += "\"" + node_id + "\":f0 ->" + "\"" + btype->getUniqueID() + "\";\n";

    for(int i = 0; i < var_def_list.size(); i++) {
      var_def_list[i].second->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(i*2 + 1) + " ->" + "\"" + var_def_list[i].second->getUniqueID() + "\";\n";
    }
    
  }

};

class BTypeAST : public BaseAST { // BType         ::= "int";
public:
    std::string btype;
    BTypeAST(const std::string &_btype) : btype(_btype) {}
    void Dump() const override {
        std::cout << "BType { " << btype << " }";
    }

    void toDot(std::string& dot) const override  {
      // 生成当前节点的唯一标识符
      std::string node_id = getUniqueID();
      std::string node_label = "BType: " + btype;  // 直接将类型名称加入标签
      std::string node_def = node_id + " [label=\"" + node_label + "\"];\n";
      dot += node_def;
    }


};

class ConstDefAST : public BaseAST { // ConstDef      ::= IDENT {"[" ConstExp "]"} "=" ConstInitVal;
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


    void toDot(std::string& dot) const override  { // ConstDef      ::= IDENT {"[" ConstExp "]"} "=" ConstInitVal; 需要显示终结符，并单独设为一个field, 就像LValAST那样
        std::string node_id = getUniqueID();
        std::string node_def = node_id + "[label=\"<f0> IDENT: " + ident;
        for(int i = 0; i < const_exp_list.size(); i++) {
            node_def += " | <f" + std::to_string(i*3 + 1) + "> [" + " | <f" + std::to_string(i*3 + 2) + "> ConstExp" + " | <f" + std::to_string(i*3 + 3) + "> ]";
        }
        node_def += " | <f" + std::to_string(const_exp_list.size()*3 + 1) + "> = | <f" + std::to_string(const_exp_list.size()*3 + 2) + "> ConstInitVal\"];\n";
        dot += node_def;

        for(int i = 0; i < const_exp_list.size(); i++) {
            const_exp_list[i].second->toDot(dot);
            dot += "\"" + node_id + "\":f" + std::to_string(i*3 + 2) + " ->" + "\"" + const_exp_list[i].second->getUniqueID() + "\";\n";
        }

        const_init_val->toDot(dot);
        dot += "\"" + node_id + "\":f" + std::to_string(const_exp_list.size() + 2) + " ->" + "\"" + const_init_val->getUniqueID() + "\";\n";
    
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
  // VarDef        ::= IDENT {"[" ConstExp "]"} | IDENT {"[" ConstExp "]"} "=" InitVal;
  void toDot(std::string& dot) const override  {
    std::string node_id = getUniqueID();
    if(type == Type::IDENT) { // VarDef        ::= IDENT {"[" ConstExp "]"} ;
      std::string node_def = node_id + "[label=\"<f0> IDENT: " + ident;
      for(int i = 0; i < const_exp_list.size(); i++) {
        node_def += " | <f" + std::to_string(i*3 + 1) + "> [" + " | <f" + std::to_string(i*3 + 2) + "> ConstExp" + " | <f" + std::to_string(i*3 + 3) + "> ]";
      }
      node_def += "\"];\n";
      dot += node_def;

      for(int i = 0; i < const_exp_list.size(); i++) {
        const_exp_list[i].second->toDot(dot);
        dot += "\"" + node_id + "\":f" + std::to_string(i*3 + 2) + " ->" + "\"" + const_exp_list[i].second->getUniqueID() + "\";\n";
      }
    } else if(type == Type::INIT) { // VarDef        ::= IDENT {"[" ConstExp "]"} "=" InitVal;
      std::string node_def = node_id + "[label=\"<f0> IDENT: " + ident;
      for(int i = 0; i < const_exp_list.size(); i++) {
        node_def += " | <f" + std::to_string(i*3 + 1) + "> [" + " | <f" + std::to_string(i*3 + 2) + "> ConstExp" + " | <f" + std::to_string(i*3 + 3) + "> ]";
      }
      node_def += " | <f" + std::to_string(const_exp_list.size()*3 + 1) + "> = | <f" + std::to_string(const_exp_list.size()*3 + 2) + "> InitVal\"];\n";
      dot += node_def;

      for(int i = 0; i < const_exp_list.size(); i++) {
        const_exp_list[i].second->toDot(dot);
        dot += "\"" + node_id + "\":f" + std::to_string(i*3 + 2) + " ->" + "\"" + const_exp_list[i].second->getUniqueID() + "\";\n";
      }

      init_val->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(const_exp_list.size()*3 + 2) + " ->" + "\"" + init_val->getUniqueID() + "\";\n";
    } else {
      std::cerr << "VarDefAST::toDot: unknown type" << std::endl;
    }
  }

};

class InitValAST : public BaseAST { // InitVal       ::= Exp | "{" [InitVal {"," InitVal}] "}"; 单个符号例如 '{'也需要单独设置一个field 
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
    // InitVal       ::= Exp | "{" [InitVal {"," InitVal}] "}"; 需要显示终结符作为单独field
    void toDot(std::string& dot) const override {
      std::string node_id = getUniqueID();
      if(type == Type::EXP) { // InitVal       ::= Exp;
        std::string node_def = node_id + "[label=\"<f0> Exp\"];\n";
        dot += node_def;
        exp->toDot(dot);
        dot += "\"" + node_id + "\":f0 ->" + "\"" + exp->getUniqueID() + "\";\n";
      } else if(type == Type::ARRAY) { // InitVal       ::= "{" [InitVal {"," InitVal}] "}";
        if(init_val_list.size() == 0) { // InitVal       ::= "{" "}"; 为空数组
          std::string node_def = node_id + "[label=\"<f0> \\{ |<f1> \\}" + "\"];\n"; 
          dot += node_def;
        } else { // InitVal       ::= "{" InitVal {"," InitVal} "}"; 至少有一个InitVal，需要显示大括号注意使用转义符
          std::string node_def = node_id + "[label=\"<f0> \\{ | <f1> InitVal";
          for(int i = 1; i < init_val_list.size() && init_val_list.size() > 1; i++) {
            node_def += " | <f" + std::to_string(i*2 + 1) + "> , | <f" + std::to_string(i*2 + 2) + "> InitVal";
          }
          node_def += " | <f" + std::to_string(init_val_list.size()*2 + 1) + "> \\}" + "\"];\n"; // 最后一个是"}
          dot += node_def;

          for(int i = 0; i < init_val_list.size(); i++) {
            init_val_list[i].second->toDot(dot);
            dot += "\"" + node_id + "\":f" + std::to_string(i*2 + 1) + " ->" + "\"" + init_val_list[i].second->getUniqueID() + "\";\n";
          }
        }
      } else {
        std::cerr << "InitValAST::toDot: unknown type" << std::endl;
      }
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

    void toDot(std::string& dot) const override {
      std::string node_id = getUniqueID();
      if(type == Type::CONSTEXP) { // ConstInitVal  ::= ConstExp;
        std::string node_def = node_id + "[label=\"<f0> ConstExp\"];\n";
        dot += node_def;
        const_exp->toDot(dot);
        dot += "\"" + node_id + "\":f0 ->" + "\"" + const_exp->getUniqueID() + "\";\n";
      } else if(type == Type::ARRAY) { // ConstInitVal  ::= "{" [ConstInitVal {"," ConstInitVal}] "}";
        if(const_init_val_list.size() == 0) { // ConstInitVal  ::= "{" "}"; 为空数组
          std::string node_def = node_id + "[label=\"<f0> \\{ |<f1> \\}" + "\"];\n"; 
          dot += node_def;
        } else { // ConstInitVal  ::= "{" ConstInitVal {"," ConstInitVal} "}"; 至少有一个ConstInitVal，需要显示大括号注意使用转义符
          std::string node_def = node_id + "[label=\"<f0> \\{ | <f1> ConstInitVal";
          for(int i = 1; i < const_init_val_list.size() && const_init_val_list.size() > 1; i++) {
            node_def += " | <f" + std::to_string(i*2 + 1) + "> , | <f" + std::to_string(i*2 + 2) + "> ConstInitVal";
          }
          node_def += " | <f" + std::to_string(const_init_val_list.size()*2 + 1) + "> \\}" + "\"];\n"; // 最后一个是"}
          dot += node_def;

          for(int i = 0; i < const_init_val_list.size(); i++) {
            const_init_val_list[i].second->toDot(dot);
            dot += "\"" + node_id + "\":f" + std::to_string(i*2 + 1) + " ->" + "\"" + const_init_val_list[i].second->getUniqueID() + "\";\n";
          }
        }
      } else {
        std::cerr << "ConstInitValAST::toDot: unknown type" << std::endl;
      }
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

    void toDot(std::string& dot) const override {
        std::string node_id = getUniqueID();
        std::string node_def = node_id + "[label=\"<f0> Exp\"];\n";
        dot += node_def;
        exp->toDot(dot);
        dot += "\"" + node_id + "\":f0 ->" + "\"" + exp->getUniqueID() + "\";\n";
    }
};


// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST { // FuncDef       ::= FuncType IDENT "(" [FuncFParams] ")" Block;
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

// FuncType 是 BaseAST
class FuncTypeAST : public BaseAST { // FuncType      ::= "void" | "int";
public:
  std::string type;
 
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

class BlockAST : public BaseAST { // Block         ::= "{" {BlockItem} "}";
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

  void toDot(std::string& dot) const override { // BlockItem     ::= Decl | Stmt;
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

class FuncRParamsAST : public BaseAST { // FuncRParams   ::= Exp {"," Exp};
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

  void toDot(std::string& dot) const override {// FuncRParams   ::= Exp {"," Exp};
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"";
    for(int i = 0; i < exp_list.size(); i++) {
      if(i == 0) {
        node_def += "<f" + std::to_string(i*2 + 0) + "> Exp";
      } else {
        node_def += " | <f" + std::to_string(i*2 - 1) + "> , | <f" + std::to_string(i*2) + "> Exp";
      }
    }
    node_def += "\"];\n";
    dot += node_def;

    for(int i = 0; i < exp_list.size(); i++) {
      exp_list[i].second->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(i*2) + " ->" + "\"" + exp_list[i].second->getUniqueID() + "\";\n";
    } 
  }

};


