#include "base_ast.hh"

class CompUnitAST : public BaseAST { // CompUnit      ::= [CompUnit] (Decl | FuncDef);
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