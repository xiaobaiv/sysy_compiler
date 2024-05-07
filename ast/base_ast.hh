#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
enum class ListType { CONSTDEF,VARDEF ,DECL, STMT, BLOCKITEM, FUNCFPARAM, EXP, CONSTEXP, INITVAL, CONSTINITVAL };

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;

  //virtual void toDot(std::string& dot) const = 0;

  // 为了区分, 我们为每个 AST 节点生成一个唯一的 ID
  virtual std::string getUniqueID() const {
        return "Node_" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
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
