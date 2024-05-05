#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
enum class ListType { CONSTDEF,VARDEF ,DECL, STMT, BLOCKITEM, FUNCFPARAM, EXP };

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
  enum class Type { FUNCDEF, COMPUNIT } type;
  std::unique_ptr<BaseAST> other_comp_unit;
  std::unique_ptr<BaseAST> func_def;
  void Dump() const override {
    std::cout << "CompUnitAST { ";
    switch (type) {
      case Type::FUNCDEF:
        func_def->Dump();
        break;
      case Type::COMPUNIT:
        other_comp_unit->Dump();
        std::cout << ", ";
        func_def->Dump();
        break;
      default:
        break;
    }
    std::cout << " }";
  }
}; 

// CompUnit 是 BaseAST
/* class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> other_comp_unit;

  void Dump() const override {
    std::cout << "CompUnitAST { ";
    other_comp_unit->Dump();
    std::cout << " }";
  }
}; */



/* 
Decl          ::= ConstDecl;
ConstDecl     ::= "const" BType ConstDefList  ";";
ConstDefList  ::= ConstDefList "," ConstDef | ConstDef ;
BType         ::= "int";
ConstDef      ::= IDENT "=" ConstInitVal;
ConstInitVal  ::= ConstExp;

Block         ::= "{" {BlockItem} "}";
BlockItem     ::= Decl | Stmt;

LVal          ::= IDENT;
PrimaryExp    ::= "(" Exp ")" | LVal | Number;

ConstExp      ::= Exp;
 */

typedef std::vector<std::pair<ListType, std::unique_ptr<BaseAST>>> List;
