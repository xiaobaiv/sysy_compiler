#pragma once
#include <string>
#include <iostream>
#include <memory>
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
  }
};

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

// ExpAST
class ExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> add_exp;

  void Dump() const override {
    std::cout << "ExpAST { ";
    add_exp->Dump();
    std::cout << " }";
  }
};

// UnaryExpAST 
class UnaryExpAST : public BaseAST {
public:
  enum class Type { PRIMARY, OP } type;
  std::unique_ptr<BaseAST> son_exp;
  std::string op;

  UnaryExpAST(std::unique_ptr<BaseAST> &_primary_exp) {
    type = Type::PRIMARY;
    son_exp = std::move(_primary_exp);
  }
  UnaryExpAST(std::string &_op, std::unique_ptr<BaseAST> &_unary_exp) {
    type = Type::OP;
    op = _op;
    son_exp = std::move(_unary_exp);
  }

  void Dump() const override {
    std::cout << "UnaryExpAST { ";
    switch (type) {
      case Type::PRIMARY:
        son_exp->Dump();
        break;
      case Type::OP:
        std::cout << op << ", " ;
        son_exp->Dump();
        break;
      default:
        break;
    }
    std::cout << " }";
  }
};

// PrimaryExpAST
class PrimaryExpAST : public BaseAST {
public:
  enum class Type { EXP, NUMBER } type;
  std::unique_ptr<BaseAST> exp;
  int number;
  PrimaryExpAST(std::unique_ptr<BaseAST> &_exp) {
    type = Type::EXP;
    exp = std::move(_exp);
  }
  PrimaryExpAST(int _number) {
    type = Type::NUMBER;
    number = _number;
  }

  void Dump() const override {
    std::cout << "PrimaryExpAST { ";
    switch (type) {
      case Type::EXP:
        exp->Dump();
        break;
      case Type::NUMBER:
        std::cout << number;
      default:
        break;
    }
    std::cout << " }";
  }
};

class MulExpAST : public BaseAST {
public:
  enum class Type { UNARYEXP, MULEXP} type;
  std::unique_ptr<BaseAST> unary_exp;
  std::unique_ptr<BaseAST> mul_exp;
  std::string op;

  MulExpAST(std::unique_ptr<BaseAST> &_unary_exp) {
    type = Type::UNARYEXP;
    unary_exp = std::move(_unary_exp);
  }
  MulExpAST(std::unique_ptr<BaseAST> &_mul_exp, std::string &_op, std::unique_ptr<BaseAST> &_unary_exp) {
    type = Type::MULEXP;
    mul_exp = std::move(_mul_exp);
    op = _op;
    unary_exp = std::move(_unary_exp);
  }

  void Dump() const override {
      std::cout << "MulExpAST { ";
      switch (type)
      {
      case Type::UNARYEXP:
        unary_exp->Dump();
        break;
      case Type::MULEXP:
        mul_exp->Dump();
        std::cout << ", " << op << ", ";
        unary_exp->Dump();
        break;
      default:
        break;
      }
      std::cout << " }";
    }
};

class AddExpAST : public BaseAST {
public:
  enum class Type { MULEXP, ADDEXP} type;
  std::unique_ptr<BaseAST> mul_exp;
  std::unique_ptr<BaseAST> add_exp;
  std::string op;

  AddExpAST(std::unique_ptr<BaseAST> &_mul_exp) {
    type = Type::MULEXP;
    mul_exp = std::move(_mul_exp);
  }
  AddExpAST(std::unique_ptr<BaseAST> &_add_exp, std::string &_op, std::unique_ptr<BaseAST> &_mul_exp) {
    type = Type::ADDEXP;
    add_exp = std::move(_add_exp);
    op = _op;
    mul_exp = std::move(_mul_exp);
  }

  void Dump() const override {
    std::cout << "AddExpAST { ";
    switch (type)
    {
    case Type::MULEXP:
      mul_exp->Dump();
      break;
    case Type::ADDEXP:
      add_exp->Dump();
      std::cout << ", " << op << ", ";
      mul_exp->Dump();
      break;
    default:
      break;
    }
    std::cout << " }";
  }
};