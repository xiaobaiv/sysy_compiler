#pragma once
#include "base_ast.hh"

class DeclAST : public BaseAST { // Decl          ::= ConstDecl | VarDecl;
 public:

    std::unique_ptr<BaseAST> const_or_var_decl;
    enum class Type { CONST, VAR } type;

    DeclAST(std::unique_ptr<BaseAST> &_const_or_var_decl, Type _type) : const_or_var_decl(std::move(_const_or_var_decl)), type(_type) {}
    ret_value_t toIR(std::string &ir) override {
        if(type == Type::CONST) { // Decl          ::= ConstDecl;
            return const_or_var_decl->toIR(ir);
        } else if(type == Type::VAR) { // Decl          ::= VarDecl;
            return const_or_var_decl->toIR(ir);
        } else {
            std::cerr << "DeclAST: unknown type" << std::endl;
            assert(0);
        }
    }
    
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
  ret_value_t toIR(std::string &ir) override {
    // 遍历ConstDef，生成IR指令
    for(auto &item : const_def_list) {
      item.second->toIR(ir);
    }
    return {0, RetType::VOID};
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
        node_def += " | <f" + std::to_string(i*2 + 1) + "> , | <f" + std::to_string(i*2+2) + "> ConstDef";
      }
    }
    node_def += " | <f" + std::to_string(const_def_list.size()*2 + 1) + "> ;\"];\n";
    dot += node_def;
    // std::cout << node_def<<std::endl;
    btype->toDot(dot);
    dot += "\"" + node_id + "\":f1 ->" + "\"" + btype->getUniqueID() + "\";\n";

    for(int i = 0; i < const_def_list.size(); i++) {
      const_def_list[i].second->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(i*2 + 2) + " ->" + "\"" + const_def_list[i].second->getUniqueID() + "\";\n";
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
  
    ret_value_t toIR(std::string &ir) override {
        if(const_exp_list.size() == 0) { // ConstDef      ::= IDENT {"[" ConstExp "]"} "=" ConstInitVal;
            // 插入符号表，简单情况，没有数组,不生成指令，只插入符号表
           symbol_table.insert(ident, {Item::Type::CONST, const_init_val->calc()});
        } else {
            std::vector<int> array_size;

            for(auto &item : const_exp_list) {
                array_size.push_back(item.second->calc());
            }
            symbol_table.insert(ident, {Item::Type::CARRAY, static_cast<int>(array_size.size())});

            // 
            int length = 1;
            for(auto &item : array_size) {
                length *= item;
            }
            std::vector<int> init_list(length, 0);
            const_init_val->calc(init_list, 0, length, array_size, static_cast<int>(array_size.size()));

            // 向 ir 中添加初始化数组的指令
            ir += allocArrayIR(ident, array_size, init_list);
        }
        return {0, RetType::VOID};
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

class ConstInitValAST : public BaseAST { // ConstInitVal  ::= ConstExp | "{" [ConstInitVal {"," ConstInitVal}] "}";
public:
    enum class Type { CONSTEXP, ARRAY } type;
    std::unique_ptr<BaseAST> const_exp;
    List const_init_val_list;

    ConstInitValAST(std::unique_ptr<BaseAST> &_const_exp) : const_exp(std::move(_const_exp)) { 
      type = Type::CONSTEXP;  
    }
    ConstInitValAST(List &_const_init_val_list) {
        for(auto &item : _const_init_val_list) {
            const_init_val_list.push_back(std::make_pair(item.first, std::move(item.second)));
        }
        type = Type::ARRAY;
    }
    int calc() override {
        if(type == Type::CONSTEXP) {
            return const_exp->calc();
        } else {
            std::cerr << "ConstInitValAST: array not supported" << std::endl;
            assert(0);
        }
    }
    bool isArray() override {
        return type == Type::ARRAY;
    }

    void calc(std::vector<int> &init_list , int start_index, int end_index, std::vector<int> &array_size, int sub_array_size ) override { // 一定是数组
        int ptr = 0;
        for(auto &item : const_init_val_list) {
          if(item.second->isArray()) {
            // 判断{}的层次
            if(ptr == 0) {
              /* 
              ** 1. new_start_index = start_index;
              ** 2. new_sub_array_size = sub_array_size - 1;
              ** 3. new_end_index = start_index + array_size[array_size.size() - 1] * array_size[array_size.size() - 2] * ... * array_size[array_size.size() - new_sub_array_size];
              ** 4. item
              ** 
                */ 
              int new_start_index = start_index;
              int new_sub_array_size = sub_array_size - 1;
              int length = 1;
              for(int i = array_size.size() - 1; i >= array_size.size() - new_sub_array_size; i--) {
                length *= array_size[i];
              }
              int new_end_index = start_index + length;
              item.second->calc(init_list, new_start_index, new_end_index, array_size, new_sub_array_size);
              
              ptr += length;
              // std::cout << ptr<<std::endl;
              // std::cout << "length = " << length << " new_start_index = " << new_start_index << " new_end_index = " << new_end_index << " new_sub_array_size = " << new_sub_array_size << std::endl;
            }
            else { // ptr > 0, 自 array_size 的最后一个元素开始，判断到最长可以整除的长度
              int new_sub_array_size = 0;
              int tmp_ptr = ptr;
              while(tmp_ptr != 0 && array_size.size() - new_sub_array_size - 1 >= 0 && (tmp_ptr) % array_size[array_size.size() - new_sub_array_size - 1] == 0) {
                new_sub_array_size++;
                tmp_ptr /= array_size[array_size.size() - new_sub_array_size - 1];
              }
              // 有可能列表初始化时非法的，也就是说ptr不是array_size的整数倍
              if(new_sub_array_size == 0) {
                std::cerr << "ConstInitValAST: array initialization error" << std::endl;
                assert(0);
              }

              int new_start_index = start_index + ptr;
              int length = 1;
              for(int i = array_size.size() - 1; i >= array_size.size() - new_sub_array_size; i--) {
                length *= array_size[i];
              }
              int new_end_index = new_start_index + length;
              item.second->calc(init_list, new_start_index, new_end_index, array_size, new_sub_array_size);
              ptr += length;
              // std::cout << ptr<<std::endl;
              // std::cout << "length = " << length << " new_start_index = " << new_start_index << " new_end_index = " << new_end_index << " new_sub_array_size = " << new_sub_array_size << std::endl;
            }

          } 
          else {
            init_list[start_index + ptr] = item.second->calc();
            ptr++;
          }
        }
        
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
  ret_value_t toIR(std::string &ir) override {
    // 遍历VarDef，生成IR指令
    for(auto &item : var_def_list) {
      item.second->toIR(ir);
    }
    return {0, RetType::VOID};
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

class VarDefAST : public BaseAST { // VarDef        ::= IDENT {"[" ConstExp "]"} | IDENT {"[" ConstExp "]"} "=" InitVal;
public:
  enum class Type { IDENT, INIT} type;
  std::string ident;
  List const_exp_list;
  std::unique_ptr<BaseAST> init_val;

  ret_value_t toIR(std::string &ir) override {
    if(type == Type::IDENT) { // VarDef        ::= IDENT {"[" ConstExp "]"} ;
      // 插入符号表，简单情况，没有数组
        if(const_exp_list.size() == 0) {
          if(!symbol_table.insert(ident, {Item::Type::VAR, -1})) {
            std::cerr << "VarDefAST: redefined variable" << std::endl;
            assert(0);
          }
          ir += allocIR(ident);
          // 检查是否是全局变量，如果是全局变量，需要初始化为0
          if(symbol_table.isGlobal()) {
            ir += "zeroinit\n";
          }
        } else { // 数组
          if(!symbol_table.insert(ident, {Item::Type::VARRAY, static_cast<int>(const_exp_list.size())})) {
            std::cerr << "VarDefAST: redefined variable" << std::endl;
            assert(0);
          }
          std::vector<int> array_size;
          for(auto &item : const_exp_list) {
            array_size.push_back(item.second->calc());
          }
          ir += allocArrayIR(ident, array_size);
        }

      

    } else if(type == Type::INIT) { // VarDef        ::= IDENT {"[" ConstExp "]"} "=" InitVal;
      // 插入符号表，简单情况，没有数组
      if(const_exp_list.size() == 0) {
        if(!symbol_table.insert(ident, {Item::Type::VAR, -1})) {
          std::cerr << "VarDefAST: redefined variable" << std::endl;
          assert(0);
        }
        ir += allocIR(ident);
        // 检查是否是全局变量，如果是全局变量，不使用storeIR
        ret_value_t ret = init_val->toIR(ir);
        if(symbol_table.isGlobal()) {
          if(ret.second == RetType::NUMBER) {
            ir += std::to_string(ret.first.number) + "\n";
          } else {
            std::cerr << "VarDefAST: Global variables can only be initialized by constants." << std::endl;
          }
        } else {
          ir += storeIR(ret, {ident, RetType::IDENT});
        }
      } else { // 数组
        if(!symbol_table.insert(ident, {Item::Type::VARRAY, static_cast<int>(const_exp_list.size())})) {
          std::cerr << "VarDefAST: redefined variable" << std::endl;
          assert(0);
        }
        std::vector<int> array_size;
        for(auto &item : const_exp_list) {
          array_size.push_back(item.second->calc());
        }
        int length = 1;
        for(auto &item : array_size) {
            length *= item;
        }
        std::vector<int> init_list(length, 0);
        init_val->calc(init_list, 0, length, array_size, array_size.size());
        ir += allocArrayIR(ident, array_size, init_list);
      }
    } else {
      std::cerr << "VarDefAST: unknown type" << std::endl;
      assert(0);
    }
    return {0, RetType::VOID};
  }


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

class InitValAST : public BaseAST { // InitVal       ::= Exp | "{" [InitVal {"," InitVal}] "}"; 
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
    bool isArray() override {
        return type == Type::ARRAY;
    }
    int calc() override {
        if(type == Type::EXP) {
            return exp->calc();
        } else {
            std::cerr << "InitValAST: array not supported" << std::endl;
            assert(0);
        }
    }
    void calc(std::vector<int> &init_list , int start_index, int end_index, std::vector<int> &array_size, int sub_array_size ) override { // 一定是数组
        int ptr = 0;
        for(auto &item : init_val_list) {
          if(item.second->isArray()) {
            // 判断{}的层次
            if(ptr == 0) {
              /* 
              ** 1. new_start_index = start_index;
              ** 2. new_sub_array_size = sub_array_size - 1;
              ** 3. new_end_index = start_index + array_size[array_size.size() - 1] * array_size[array_size.size() - 2] * ... * array_size[array_size.size() - new_sub_array_size];
              ** 4. item
              ** 
                */ 
              int new_start_index = start_index;
              int new_sub_array_size = sub_array_size - 1;
              int length = 1;
              for(int i = array_size.size() - 1; i >= array_size.size() - new_sub_array_size; i--) {
                length *= array_size[i];
              }
              int new_end_index = start_index + length;
              item.second->calc(init_list, new_start_index, new_end_index, array_size, new_sub_array_size);
              
              ptr += length;
              // std::cout << ptr<<std::endl;
              // std::cout << "length = " << length << " new_start_index = " << new_start_index << " new_end_index = " << new_end_index << " new_sub_array_size = " << new_sub_array_size << std::endl;
            }
            else { // ptr > 0, 自 array_size 的最后一个元素开始，判断到最长可以整除的长度
              int new_sub_array_size = 0;
              int tmp_ptr = ptr;
              while(tmp_ptr != 0 && array_size.size() - new_sub_array_size - 1 >= 0 && (tmp_ptr) % array_size[array_size.size() - new_sub_array_size - 1] == 0) {
                new_sub_array_size++;
                tmp_ptr /= array_size[array_size.size() - new_sub_array_size - 1];
              }
              // 有可能列表初始化时非法的，也就是说ptr不是array_size的整数倍
              if(new_sub_array_size == 0) {
                std::cerr << "ConstInitValAST: array initialization error" << std::endl;
                assert(0);
              }

              int new_start_index = start_index + ptr;
              int length = 1;
              for(int i = array_size.size() - 1; i >= array_size.size() - new_sub_array_size; i--) {
                length *= array_size[i];
              }
              int new_end_index = new_start_index + length;
              item.second->calc(init_list, new_start_index, new_end_index, array_size, new_sub_array_size);
              ptr += length;
              // std::cout << ptr<<std::endl;
              // std::cout << "length = " << length << " new_start_index = " << new_start_index << " new_end_index = " << new_end_index << " new_sub_array_size = " << new_sub_array_size << std::endl;
            }

          } 
          else {
            init_list[start_index + ptr] = item.second->calc();
            ptr++;
          }
        }
        
    }

    ret_value_t toIR(std::string &ir) override {
        if(type == Type::EXP) {
            return exp->toIR(ir);
        } else {
            std::cerr << "InitValAST: array not supported" << std::endl;
            assert(0);
        }
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
