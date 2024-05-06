#include "base_ast.hh"
// ExpAST
class ExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lor_exp;

  void Dump() const override {
    std::cout << "ExpAST { ";
    lor_exp->Dump();
    std::cout << " }";
  }
};

// UnaryExpAST 
class UnaryExpAST : public BaseAST {
public:
  enum class Type { PRIMARY, OP , IDENT} type;
  enum class Option {F0, F1} option;
  std::unique_ptr<BaseAST> son_exp;
  std::string op;
  std::string ident;
  std::unique_ptr<BaseAST> func_r_params;

  UnaryExpAST(std::unique_ptr<BaseAST> &_primary_exp) {
    type = Type::PRIMARY;
    son_exp = std::move(_primary_exp);
  }
  UnaryExpAST(std::string &_op, std::unique_ptr<BaseAST> &_unary_exp) {
    type = Type::OP;
    op = _op;
    son_exp = std::move(_unary_exp);
  }
  UnaryExpAST() {}
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
      case Type::IDENT:
        std::cout << ident;
        if (option == Option::F1) {
          std::cout << "(";
          func_r_params->Dump();
          std::cout << ")";
        }
        break;
      default:
        break;
    }
    std::cout << " }";
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

class LValAST : public BaseAST {
public:

  enum class Option { EXP0, EXP1 } option;

  std::string ident;
  std::unique_ptr<BaseAST> exp;

  LValAST(std::string &_ident) : ident(_ident) {
    option = Option::EXP0;
  }

  LValAST(std::string &_ident, std::unique_ptr<BaseAST> &_exp) : ident(_ident) {
    option = Option::EXP1;
    exp = std::move(_exp);
  }

  void Dump() const override {
    std::cout << "LValAST { ";
    if (option == Option::EXP0) {
      std::cout << ident;
    } else {
      std::cout << ident << "[";
      exp->Dump();
      std::cout << "]";
    }
    std::cout << " }";
  }

};

// PrimaryExpAST
class PrimaryExpAST : public BaseAST {
public:
  enum class Type { EXP, NUMBER, LVAL } type;
  std::unique_ptr<BaseAST> exp_or_lval;
  int number;
  PrimaryExpAST() {}
  PrimaryExpAST(int _number) {
    type = Type::NUMBER;
    number = _number;
  }

  void Dump() const override {
    std::cout << "PrimaryExpAST { ";
    switch (type) {
      case Type::EXP:
        exp_or_lval->Dump();
        break;
      case Type::NUMBER:
        std::cout << number;
        break;
      case Type::LVAL:
        exp_or_lval->Dump();
        break;
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

class RelExpAST : public BaseAST {
public:
  enum class Type {ADDEXP, RELEXP} type;
  std::unique_ptr<BaseAST> add_exp;
  std::unique_ptr<BaseAST> rel_exp;
  std::string op;
  
  RelExpAST(std::unique_ptr<BaseAST> &_add_exp) {
    type = Type::ADDEXP;
    add_exp = std::move(_add_exp);
  }
  RelExpAST(std::unique_ptr<BaseAST> &_rel_exp, std::string &_op, std::unique_ptr<BaseAST> &_add_exp) {
    type = Type::RELEXP;
    rel_exp = std::move(_rel_exp);
    op = _op;
    add_exp = std::move(_add_exp);
  }

  void Dump() const override {
    std::cout << "RelExpAST { ";
    switch (type)
    {
    case Type::ADDEXP:
      add_exp->Dump();
      break;
    case Type::RELEXP:
      rel_exp->Dump();
      std::cout << ", " << op << ", ";
      rel_exp->Dump();
    default:
      break;
    }
    std::cout << " }";
  }
};

class EqExpAST : public BaseAST {
public:
  enum class Type {RELEXP, EQEXP} type;
  std::unique_ptr<BaseAST> rel_exp;
  std::unique_ptr<BaseAST> eq_exp;
  std::string op;

  EqExpAST(std::unique_ptr<BaseAST> &_rel_exp) {
    type = Type::RELEXP;
    rel_exp = std::move(_rel_exp);
  }
  EqExpAST(std::unique_ptr<BaseAST> &_eq_exp, std::string &_op, std::unique_ptr<BaseAST> &_rel_exp) {
    type = Type::EQEXP;
    eq_exp = std::move(_eq_exp);
    op = _op;
    rel_exp = std::move(_rel_exp);
  }

  void Dump() const override {
    std::cout << "EqExpAST { ";
    switch (type)
    {
    case Type::RELEXP:
      rel_exp->Dump();
      break;
    case Type::EQEXP:
      eq_exp->Dump();
      std::cout << ", " << op << ", ";
      rel_exp->Dump();
    default:
      break;
    }
    std::cout << " }";
  }
};

class LAndExpAST : public BaseAST {
public:
  enum class Type {EQEXP, LANDEXP} type;
  std::unique_ptr<BaseAST> eq_exp;
  std::unique_ptr<BaseAST> land_exp;
  std::string op;

  LAndExpAST(std::unique_ptr<BaseAST> &_eq_exp) {
    type = Type::EQEXP;
    eq_exp = std::move(_eq_exp);
  }
  LAndExpAST(std::unique_ptr<BaseAST> &_land_exp, std::string &_op, std::unique_ptr<BaseAST> &_eq_exp) {
    type = Type::LANDEXP;
    land_exp = std::move(_land_exp);
    op = _op;
    eq_exp = std::move(_eq_exp);
  }

  void Dump() const override {
    std::cout << "LandExpAST { ";
    switch (type)
    {
    case Type::EQEXP:
      eq_exp->Dump();
      break;
    case Type::LANDEXP:
      land_exp->Dump();
      std::cout << ", " << op << ", ";
      eq_exp->Dump();
    default:
      break;
    }
    std::cout << " }";
  }
};

class LOrExpAST : public BaseAST {
public:
  enum class Type {LANDEXP, LOREXP} type;
  std::unique_ptr<BaseAST> land_exp;
  std::unique_ptr<BaseAST> lor_exp;
  std::string op;

  LOrExpAST(std::unique_ptr<BaseAST> &_land_exp) {
    type = Type::LANDEXP;
    land_exp = std::move(_land_exp);
  }
  LOrExpAST(std::unique_ptr<BaseAST> &_lor_exp, std::string &_op, std::unique_ptr<BaseAST> &_land_exp) {
    type = Type::LOREXP;
    lor_exp = std::move(_lor_exp);
    op = _op;
    land_exp = std::move(_land_exp);
  }

  void Dump() const override {
    std::cout << "LorExpAST { ";
    switch (type)
    {
    case Type::LANDEXP:
      land_exp->Dump();
      break;
    case Type::LOREXP:
      lor_exp->Dump();
      std::cout << ", " << op << ", ";
      land_exp->Dump();
    default:
      break;
    }
    std::cout << " }";
  }
};