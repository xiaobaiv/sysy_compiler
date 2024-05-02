#include "base_ast.hh"

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump() const override {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout<< ", " << ident << ", ";
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

// Block 继承 BaseAST
class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override {
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout << " }";
  }
};

// Stmt 
class StmtAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override {
    std::cout << "StmtAST { ";
    exp->Dump();
    std::cout << " }";
  }
};

