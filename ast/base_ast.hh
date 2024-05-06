#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
enum class ListType { CONSTDEF,VARDEF ,DECL, STMT, BLOCKITEM, FUNCFPARAM, EXP, CONSTEXP };

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
