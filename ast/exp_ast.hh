#pragma once
#include "base_ast.hh"

class ExpAST : public BaseAST
{ // Exp           ::= LOrExp;
public:
  std::unique_ptr<BaseAST> lor_exp;

  ret_value_t toIR(std::string &ir) override
  {
    return lor_exp->toIR(ir);
  }
  int calc() override
  {
    return lor_exp->calc();
  }
  void Dump() const override
  {
    std::cout << "ExpAST { ";
    lor_exp->Dump();
    std::cout << " }";
  }
  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"<f0> LOrExp\"];\n";
    dot += node_def;
    lor_exp->toDot(dot);
    dot += "\"" + node_id + "\":f0 ->" + "\"" + lor_exp->getUniqueID() + "\";\n";
  }
};

class LValAST : public BaseAST
{ // LVal          ::= IDENT {"[" Exp "]"};
public:
  std::string ident;
  List exp_list;

  LValAST(std::string &_ident, List &_exp_list)
  {
    ident = _ident;
    for (auto &exp : _exp_list)
    {
      exp_list.push_back(std::make_pair(exp.first, std::move(exp.second)));
    }
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (symbol_table.find(ident) == nullptr)
    {
      std::cerr << "LValAST::toIR: undefined ident " << ident << std::endl;
      assert(0);
    }
    // 直接用到LVAL的语法：表达式PrimaryAst, stmt 中的赋值语句
    // Ident 有可能是数组，也有可能是指针，也有可能是常量，也有可能是变量。
    // 对于数组，需要判断返回的是数组的元素还是数组的指针
    /* 1. 首先判断ident的类型 */
    if (symbol_table.isConst(ident))
    {
      // 如果是 常量，直接返回常量的值
      return {symbol_table.getValue(ident), RetType::NUMBER};
    }
    if (symbol_table.isVar(ident))
    {
      // 如果是 变量，返回变量的地址，由caller决定是否需要load
      return {ident, RetType::IDENT};
    }
    if (symbol_table.isArray(ident))
    {
      std::cout << ident << " is an array" << std::endl;
      std::vector<ret_value_t> indexs = {};
      for (auto &exp : exp_list)
      {
        indexs.push_back(exp.second->toIR(ir));
      }
      ir += getelemptrIR(ident, indexs);
      if (exp_list.size() == 0)
      {
        ir += "\t%ptr" + std::to_string(global_ptr_index++) + " = getelemptr @" + symbol_table.getUniqueIdent(ident) + ", 0\n";
        return {global_ptr_index - 1, RetType::ARRAYPTR};
      }
      // 如果是 数组, 先判断需要返回的是数组的元素还是数组的解引用
      if (exp_list.size() < symbol_table.getValue(ident))
      {

        // 如果exp_list.size() < symbol_table.getValue(ident)，说明是数组的解引用，
        // 返回地址
        ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getelemptr %ptr" + std::to_string(global_ptr_index - 1) + ", 0\n";
        global_ptr_index++;
        return {global_ptr_index - 1, RetType::ARRAYPTR};
      }
      else if (exp_list.size() == symbol_table.getValue(ident))
      {
        return {global_ptr_index - 1, RetType::ELEMENTPTR};
      }
      else
      {
        std::cerr << "LValAST::toIR: array index out of range" << std::endl;
        assert(0);
      }
    }
    if (symbol_table.isPtr(ident))
    {
      std::cout << ident << " is a pointer" << std::endl;
      std::vector<ret_value_t> indexs = {};
      for (auto &exp : exp_list)
      {
        indexs.push_back(exp.second->toIR(ir));
      }
      ir += getptrIR(ident, indexs);
      if (exp_list.size() == 0)
      {
        return {global_ptr_index - 1, RetType::PTR};
      }
      if (exp_list.size() < symbol_table.getValue(ident))
      {
        ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getelemptr %ptr" + std::to_string(global_ptr_index - 1) + ", 0\n";
        global_ptr_index++;
        return {global_ptr_index - 1, RetType::ARRAYPTR};
      }
      else if (exp_list.size() == symbol_table.getValue(ident))
      {
        return {global_ptr_index - 1, RetType::ELEMENTPTR};
      }
      else
      {
        std::cerr << "LValAST::toIR: array index out of range" << std::endl;
        assert(0);
      }
    }
    return {1, RetType::VOID};
  }
  int calc() override
  {
    // 在符号表中查找标识符， TODO,目前的符号表很简单，只有一个全局符号表，并且只有常量
    // 符号表 static std::unordered_map<std::string, int> symbol_table;
    if (exp_list.size() == 0)
    {
      if (symbol_table.find(ident) == nullptr)
      {
        std::cerr << "LValAST::calc: undefined ident " << ident << std::endl;
        assert(0);
      }
      if (symbol_table.isConst(ident))
      {
        return symbol_table.getValue(ident);
      }
      else
      {
        std::cerr << "LValAST::calc: not const" << std::endl;
        assert(0);
      }
    }
    else
    {
      std::cerr << "LValAST::calc: array not supported" << std::endl;
      assert(0);
    }
  }
  void Dump() const override
  {
    std::cout << "LValAST { " << ident;
    for (auto &exp : exp_list)
    {
      std::cout << "[";
      exp.second->Dump();
      std::cout << "]";
    }
    std::cout << " }";
  }

  void toDot(std::string &dot) const override
  { // LVal          ::= IDENT {"[" Exp "]"}; 需要显示扩号，并单独设为一个field, 就像LValAST那样
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"<f0> IDENT: " + ident;
    for (int i = 0; i < exp_list.size(); i++)
    {
      node_def += " | <f" + std::to_string(i * 3 + 1) + "> \\[" + " | <f" + std::to_string(i * 3 + 2) + "> Exp | <f" + std::to_string(i * 3 + 3) + "> \\]";
    }
    node_def += "\"];\n";
    dot += node_def;
    for (int i = 0; i < exp_list.size(); i++)
    {
      exp_list[i].second->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(i * 3 + 2) + " ->" + "\"" + exp_list[i].second->getUniqueID() + "\";\n";
    }
  }
};

class PrimaryExpAST : public BaseAST
{ // PrimaryExp    ::= "(" Exp ")" | LVal | Number;
public:
  enum class Type
  {
    EXP,
    NUMBER,
    LVAL
  } type;
  std::unique_ptr<BaseAST> exp_or_lval;
  int number;
  PrimaryExpAST() {}
  PrimaryExpAST(int _number)
  {
    type = Type::NUMBER;
    number = _number;
  }
  int calc() override
  {
    if (type == Type::EXP)
    {
      return exp_or_lval->calc();
    }
    else if (type == Type::NUMBER)
    {
      return number;
    }
    else if (type == Type::LVAL)
    {
      return exp_or_lval->calc();
    }
    else
    {
      std::cerr << "PrimaryExpAST::calc: unknown type" << std::endl;
      assert(0);
    }
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::EXP)
    {
      return exp_or_lval->toIR(ir);
    }
    else if (type == Type::NUMBER)
    {
      return {RetValue{number}, RetType::NUMBER};
    }
    else if (type == Type::LVAL)
    {
      ret_value_t ret = exp_or_lval->toIR(ir);
      if (ret.second == RetType::NUMBER)
        return ret;
      if (ret.second == RetType::IDENT)
      {
        ir += loadIR(ret);
        return {global_var_index - 1, RetType::INDEX};
      }
      if (ret.second == RetType::ARRAY)
      {
        // 不需要load
        return ret;
      }
      if (ret.second == RetType::ARRAYPTR)
      {
        // 不需要load
        return ret;
      }
      if (ret.second == RetType::ELEMENTPTR)
      {
        ir += loadIR(ret);
        return {global_var_index - 1, RetType::INDEX};
      }
      if (ret.second == RetType::PTR)
      {
        return ret;
      }
      assert(0);
    }
    else
    {
      std::cerr << "PrimaryExpAST::toIR: unknown type" << std::endl;
    }
    return {1, RetType::VOID};
  } // PrimaryExp    ::= "(" Exp ")" | LVal | Number;  Exp ::= LOrExp; LOrExp ::= LAndExp | LOrExp "||" LAndExp; LAndExp ::= EqExp | LAndExp "&&" EqExp; EqExp ::= RelExp | EqExp ("==" | "!=") RelExp; RelExp ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp; AddExp ::= MulExp | AddExp ("+" | "-") MulExp; MulExp ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp; UnaryExp ::= PrimaryExp | IDENT "(" [FuncRParams] ")" | UnaryOp UnaryExp; UnaryOp ::= "+" | "-" | "!";

  void Dump() const override
  {
    std::cout << "PrimaryExpAST { ";
    switch (type)
    {
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

  void toDot(std::string &dot) const override
  { // PrimaryExp    ::= "(" Exp ")" | LVal | Number;
    std::string node_id = getUniqueID();
    if (type == Type::EXP)
    { // PrimaryExp    ::= "(" Exp ")";
      std::string node_def = node_id + "[label=\"<f0> \\( | <f1> Exp | <f2> \\)\"];\n";
      dot += node_def;
      exp_or_lval->toDot(dot);
      dot += "\"" + node_id + "\":f1 ->" + "\"" + exp_or_lval->getUniqueID() + "\";\n";
    }
    else if (type == Type::NUMBER)
    { // PrimaryExp    ::= Number;
      std::string node_def = node_id + "[label=\"<f0> Number: " + std::to_string(number) + "\"];\n";
      dot += node_def;
    }
    else if (type == Type::LVAL)
    { // PrimaryExp    ::= LVal;
      std::string node_def = node_id + "[label=\"<f0> LVal\"];\n";
      dot += node_def;
      exp_or_lval->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + exp_or_lval->getUniqueID() + "\";\n";
    }
    else
    {
      std::cerr << "PrimaryExpAST::toDot: unknown type" << std::endl;
    }
  }
};

class UnaryExpAST : public BaseAST
{ // UnaryExp      ::= PrimaryExp | IDENT "(" [FuncRParams] ")" | UnaryOp UnaryExp;
public:
  enum class Type
  {
    PRIMARY,
    OP,
    IDENT
  } type;
  enum class Option
  {
    F0,
    F1
  } option;
  std::unique_ptr<BaseAST> son_exp;
  std::string op;
  std::string ident;
  std::unique_ptr<BaseAST> func_r_params;

  UnaryExpAST(std::unique_ptr<BaseAST> &_primary_exp)
  {
    type = Type::PRIMARY;
    son_exp = std::move(_primary_exp);
  }
  UnaryExpAST(std::string &_op, std::unique_ptr<BaseAST> &_unary_exp)
  {
    type = Type::OP;
    op = _op;
    son_exp = std::move(_unary_exp);
  }
  UnaryExpAST() {}
  int calc() override
  {
    if (type == Type::PRIMARY)
    {
      return son_exp->calc();
    }
    else if (type == Type::OP)
    {
      if (op == "-")
      {
        return -son_exp->calc();
      }
      else if (op == "!")
      {
        return !son_exp->calc();
      }
      else if (op == "+")
      {
        return son_exp->calc();
      }
      else
      {
        std::cerr << "UnaryExpAST::calc: unknown op" << std::endl;
        assert(0);
      }
    }
    else if (type == Type::IDENT)
    {
      std::cerr << "UnaryExpAST::calc: IDENT" << std::endl;
      assert(0);
    }
    else
    {
      std::cerr << "UnaryExpAST::calc: unknown type" << std::endl;
      assert(0);
    }
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::PRIMARY)
    {
      return son_exp->toIR(ir);
    }
    else if (type == Type::OP)
    {
      if (op == "-")
      { // 变补 (取负数): 0 减去操作数
        ret_value_t ret1 = son_exp->toIR(ir);
        ir += getIR("sub", {0, RetType::NUMBER}, ret1);
        return {global_var_index - 1, RetType::INDEX};
      }
      else if (op == "!")
      { // 逻辑取反: 操作数和 0 比较相等
        ret_value_t ret1 = son_exp->toIR(ir);
        ir += getIR("eq", ret1, {0, RetType::NUMBER});
        return {global_var_index - 1, RetType::INDEX};
      }
      else if (op == "+")
      {
        return son_exp->toIR(ir);
      }
      else
      {
        std::cerr << "UnaryExpAST::toIR: unknown op" << std::endl;
        assert(0);
      }
    }
    else if (type == Type::IDENT)
    { // FunCall ::= "call" SYMBOL "(" [Value {"," Value}] ")"; ######  IDENT "(" [FuncRParams] ")"
      if (option == Option::F0)
      {
        if (symbol_table.find(ident) == nullptr)
        {
          symbol_table.print();
          std::cerr << "UnaryExpAST::toIR: undefined ident: " << ident << std::endl;
          assert(0);
        }
        if (symbol_table.isFunc(ident))
        { // IDENT "(" ")"
          ir += callIR(ident);
          // 返回值
          return {global_var_index - 1, RetType::INDEX};
        }
        else
        {
          std::cerr << "UnaryExpAST::toIR: not a function: " << ident << std::endl;
          assert(0);
        }
      }
      else if (option == Option::F1)
      {
        if (symbol_table.find(ident) == nullptr)
        {
          std::cerr << "UnaryExpAST::toIR: undefined ident: " << ident << std::endl;
          assert(0);
        }
        if (symbol_table.isFunc(ident))
        {
          std::vector<ret_value_t> args;
          func_r_params->readArgs(args);
          ir += callIR(ident, args);
          return {global_var_index - 1, RetType::INDEX};
        }
        else
        {
          std::cerr << "UnaryExpAST::toIR: not a function: " << ident << std::endl;
          assert(0);
        }
      }
      else
      {
        std::cerr << "UnaryExpAST::toIR: unknown option" << std::endl;
        assert(0);
      }
    }
    else
    {
      std::cerr << "UnaryExpAST::toIR: unknown type" << std::endl;
      assert(0);
    }
  }

  void Dump() const override
  {
    std::cout << "UnaryExpAST { ";
    switch (type)
    {
    case Type::PRIMARY:
      son_exp->Dump();
      break;
    case Type::OP:
      std::cout << op << ", ";
      son_exp->Dump();
      break;
    case Type::IDENT:
      std::cout << ident;
      if (option == Option::F1)
      {
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

  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    if (type == Type::PRIMARY)
    { // UnaryExp      ::= PrimaryExp;
      std::string node_def = node_id + "[label=\"<f0> PrimaryExp\"];\n";
      dot += node_def;
      son_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + son_exp->getUniqueID() + "\";\n";
    }
    else if (type == Type::OP)
    { // UnaryExp      ::= UnaryOp UnaryExp;
      std::string node_def = node_id + "[label=\"<f0> UnaryOp:" + op + " | <f1> UnaryExp\"];\n";
      dot += node_def;
      son_exp->toDot(dot);
      dot += "\"" + node_id + "\":f1 ->" + "\"" + son_exp->getUniqueID() + "\";\n";
    }
    else if (type == Type::IDENT)
    { // UnaryExp      ::= IDENT "(" ")";
      if (option == Option::F0)
      { // UnaryExp      ::= IDENT "(" ")"; 需要显示扩号，并单独设为一个field, 就像InitValAST那样
        std::string node_def = node_id + "[label=\"<f0> IDENT: " + ident + " | <f1> \\( | <f2> \\)\"];\n";
      }
      else if (option == Option::F1)
      { // UnaryExp      ::= IDENT "(" FuncRParams ")"; 需要显示扩号，并单独设为一个field, 就像InitValAST那样
        std::string node_def = node_id + "[label=\"<f0> IDENT: " + ident + " | <f1> \\( | <f2> FuncRParams | <f3> \\)\"];\n";
        dot += node_def;
        func_r_params->toDot(dot);
        dot += "\"" + node_id + "\":f2 ->" + "\"" + func_r_params->getUniqueID() + "\";\n";
      }
    }
    else
    {
      std::cerr << "UnaryExpAST::toDot: unknown type" << std::endl;
    }
  }
};

class FuncRParamsAST : public BaseAST
{ // FuncRParams   ::= Exp {"," Exp};
public:
  List exp_list;
  FuncRParamsAST(List &_exp_list)
  {
    for (auto &item : _exp_list)
    {
      exp_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
  }

  // 记录参数的ret_value_t, 到vector中
  void readArgs(std::vector<ret_value_t> &args) override
  {
    for (auto &item : exp_list)
    {
      args.push_back(item.second->toIR(ir));
    }
  }

  void Dump() const override
  {
    std::cout << "FuncRParamsAST { ";
    for (auto &item : exp_list)
    {
      item.second->Dump();
    }
    std::cout << " }";
  }

  void toDot(std::string &dot) const override
  { // FuncRParams   ::= Exp {"," Exp};
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"";
    for (int i = 0; i < exp_list.size(); i++)
    {
      if (i == 0)
      {
        node_def += "<f" + std::to_string(i * 2 + 0) + "> Exp";
      }
      else
      {
        node_def += " | <f" + std::to_string(i * 2 - 1) + "> , | <f" + std::to_string(i * 2) + "> Exp";
      }
    }
    node_def += "\"];\n";
    dot += node_def;

    for (int i = 0; i < exp_list.size(); i++)
    {
      exp_list[i].second->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(i * 2) + " ->" + "\"" + exp_list[i].second->getUniqueID() + "\";\n";
    }
  }
};

class MulExpAST : public BaseAST
{ // MulExp        ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
public:
  enum class Type
  {
    UNARYEXP,
    MULEXP
  } type;
  std::unique_ptr<BaseAST> unary_exp;
  std::unique_ptr<BaseAST> mul_exp;
  std::string op;

  MulExpAST(std::unique_ptr<BaseAST> &_unary_exp)
  {
    type = Type::UNARYEXP;
    unary_exp = std::move(_unary_exp);
  }
  MulExpAST(std::unique_ptr<BaseAST> &_mul_exp, std::string &_op, std::unique_ptr<BaseAST> &_unary_exp)
  {
    type = Type::MULEXP;
    mul_exp = std::move(_mul_exp);
    op = _op;
    unary_exp = std::move(_unary_exp);
  }
  int calc() override
  {
    if (type == Type::UNARYEXP)
    {
      return unary_exp->calc();
    }
    else
    {
      int i1 = mul_exp->calc();
      int i2 = unary_exp->calc();
      if (op == "*")
      {
        return i1 * i2;
      }
      else if (op == "/")
      {
        return i1 / i2;
      }
      else if (op == "%")
      {
        return i1 % i2;
      }
      else
      {
        std::cerr << "MulExpAST::calc: unknown op" << std::endl;
        assert(0);
      }
    }
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::UNARYEXP)
    {
      return unary_exp->toIR(ir);
    }
    else
    {
      ret_value_t i1 = mul_exp->toIR(ir);
      ret_value_t i2 = unary_exp->toIR(ir);
      if (op == "*")
      {
        ir += getIR("mul", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else if (op == "/")
      {
        ir += getIR("div", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else if (op == "%")
      {
        ir += getIR("mod", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else
      {
        std::cerr << "MulExpAST::toIR: unknown op" << std::endl;
        return {1, RetType::VOID};
      }
    }
  }

  void Dump() const override
  {
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

  void toDot(std::string &dot) const override
  { // MulExp        ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp; 需要显示终结符，并单独设为一个field, 就像LValAST那样
    std::string node_id = getUniqueID();
    if (type == Type::UNARYEXP)
    { // MulExp        ::= UnaryExp;
      std::string node_def = node_id + "[label=\"<f0> UnaryExp\"];\n";
      dot += node_def;
      unary_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + unary_exp->getUniqueID() + "\";\n";
    }
    else if (type == Type::MULEXP)
    { // MulExp        ::= MulExp ("*" | "/" | "%") UnaryExp;
      std::string node_def = node_id + "[label=\"<f0> MulExp | <f1> " + op + " | <f2> UnaryExp\"];\n";
      dot += node_def;
      mul_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + mul_exp->getUniqueID() + "\";\n";
      unary_exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + unary_exp->getUniqueID() + "\";\n";
    }
    else
    {
      std::cerr << "MulExpAST::toDot: unknown type" << std::endl;
    }
  }
};

class AddExpAST : public BaseAST
{ // AddExp        ::= MulExp | AddExp ("+" | "-") MulExp;
public:
  enum class Type
  {
    MULEXP,
    ADDEXP
  } type;
  std::unique_ptr<BaseAST> mul_exp;
  std::unique_ptr<BaseAST> add_exp;
  std::string op;

  AddExpAST(std::unique_ptr<BaseAST> &_mul_exp)
  {
    type = Type::MULEXP;
    mul_exp = std::move(_mul_exp);
  }
  AddExpAST(std::unique_ptr<BaseAST> &_add_exp, std::string &_op, std::unique_ptr<BaseAST> &_mul_exp)
  {
    type = Type::ADDEXP;
    add_exp = std::move(_add_exp);
    op = _op;
    mul_exp = std::move(_mul_exp);
  }
  int calc() override
  {
    if (type == Type::MULEXP)
    {
      return mul_exp->calc();
    }
    else
    {
      int i1 = add_exp->calc();
      int i2 = mul_exp->calc();
      if (op == "+")
      {
        return i1 + i2;
      }
      else if (op == "-")
      {
        return i1 - i2;
      }
      else
      {
        std::cerr << "AddExpAST::calc: unknown op" << std::endl;
        assert(0);
      }
    }
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::MULEXP)
    {
      return mul_exp->toIR(ir);
    }
    else
    {
      ret_value_t i1 = add_exp->toIR(ir);
      ret_value_t i2 = mul_exp->toIR(ir);
      if (op == "+")
      {
        ir += getIR("add", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else if (op == "-")
      {
        ir += getIR("sub", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else
      {
        std::cerr << "AddExpAST::toIR: unknown op" << std::endl;
        return {1, RetType::VOID};
      }
    }
  }

  void Dump() const override
  {
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

  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    if (type == Type::MULEXP)
    { // AddExp        ::= MulExp;
      std::string node_def = node_id + "[label=\"<f0> MulExp\"];\n";
      dot += node_def;
      mul_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + mul_exp->getUniqueID() + "\";\n";
    }
    else if (type == Type::ADDEXP)
    { // AddExp        ::= AddExp ("+" | "-") MulExp;
      std::string node_def = node_id + "[label=\"<f0> AddExp | <f1> " + op + " | <f2> MulExp\"];\n";
      dot += node_def;
      add_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + add_exp->getUniqueID() + "\";\n";
      mul_exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + mul_exp->getUniqueID() + "\";\n";
    }
    else
    {
      std::cerr << "AddExpAST::toDot: unknown type" << std::endl;
    }
  }
};

class RelExpAST : public BaseAST
{ // RelExp        ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
public:
  enum class Type
  {
    ADDEXP,
    RELEXP
  } type;
  std::unique_ptr<BaseAST> add_exp;
  std::unique_ptr<BaseAST> rel_exp;
  std::string op;

  RelExpAST(std::unique_ptr<BaseAST> &_add_exp)
  {
    type = Type::ADDEXP;
    add_exp = std::move(_add_exp);
  }
  RelExpAST(std::unique_ptr<BaseAST> &_rel_exp, std::string &_op, std::unique_ptr<BaseAST> &_add_exp)
  {
    type = Type::RELEXP;
    rel_exp = std::move(_rel_exp);
    op = _op;
    add_exp = std::move(_add_exp);
  }
  int calc() override
  {
    if (type == Type::ADDEXP)
    {
      return add_exp->calc();
    }
    else
    {
      int i1 = rel_exp->calc();
      int i2 = add_exp->calc();
      if (op == "<")
      {
        return i1 < i2;
      }
      else if (op == ">")
      {
        return i1 > i2;
      }
      else if (op == "<=")
      {
        return i1 <= i2;
      }
      else if (op == ">=")
      {
        return i1 >= i2;
      }
      else
      {
        std::cerr << "RelExpAST::calc: unknown op" << std::endl;
        return 0;
      }
    }
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::ADDEXP)
    {
      return add_exp->toIR(ir);
    }
    else
    {
      ret_value_t i1 = rel_exp->toIR(ir);
      ret_value_t i2 = add_exp->toIR(ir);
      if (op == "<")
      {
        ir += getIR("lt", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else if (op == ">")
      {
        ir += getIR("gt", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else if (op == "<=")
      {
        ir += getIR("le", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else if (op == ">=")
      {
        ir += getIR("ge", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else
      {
        std::cerr << "RelExpAST::toIR: unknown op" << std::endl;
        return {1, RetType::VOID};
      }
    }
  }

  void Dump() const override
  {
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

  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    if (type == Type::ADDEXP)
    { // RelExp        ::= AddExp;
      std::string node_def = node_id + "[label=\"<f0> AddExp\"];\n";
      dot += node_def;
      add_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + add_exp->getUniqueID() + "\";\n";
    }
    else if (type == Type::RELEXP)
    { // RelExp        ::= RelExp ("<" | ">" | "<=" | ">=") AddExp;
      std::string node_def = node_id + "[label=\"<f0> RelExp | <f1> \\" + op + " | <f2> AddExp\"];\n";
      dot += node_def;
      rel_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + rel_exp->getUniqueID() + "\";\n";
      add_exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + add_exp->getUniqueID() + "\";\n";
    }
    else
    {
      std::cerr << "RelExpAST::toDot: unknown type" << std::endl;
    }
  }
};

class EqExpAST : public BaseAST
{ // EqExp         ::= RelExp | EqExp ("==" | "!=") RelExp;
public:
  enum class Type
  {
    RELEXP,
    EQEXP
  } type;
  std::unique_ptr<BaseAST> rel_exp;
  std::unique_ptr<BaseAST> eq_exp;
  std::string op;

  EqExpAST(std::unique_ptr<BaseAST> &_rel_exp)
  {
    type = Type::RELEXP;
    rel_exp = std::move(_rel_exp);
  }
  EqExpAST(std::unique_ptr<BaseAST> &_eq_exp, std::string &_op, std::unique_ptr<BaseAST> &_rel_exp)
  {
    type = Type::EQEXP;
    eq_exp = std::move(_eq_exp);
    op = _op;
    rel_exp = std::move(_rel_exp);
  }
  int calc() override
  {
    if (type == Type::RELEXP)
    {
      return rel_exp->calc();
    }
    else
    {
      int i1 = eq_exp->calc();
      int i2 = rel_exp->calc();
      if (op == "==")
      {
        return i1 == i2;
      }
      else
      {
        return i1 != i2;
      }
    }
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::RELEXP)
    {
      return rel_exp->toIR(ir);
    }
    else
    {
      ret_value_t i1 = eq_exp->toIR(ir);
      ret_value_t i2 = rel_exp->toIR(ir);
      if (op == "==")
      {
        ir += getIR("eq", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
      else
      {
        ir += getIR("ne", i1, i2);
        return {global_var_index - 1, RetType::INDEX};
      }
    }
  }

  void Dump() const override
  {
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

  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    if (type == Type::RELEXP)
    { // EqExp         ::= RelExp;
      std::string node_def = node_id + "[label=\"<f0> RelExp\"];\n";
      dot += node_def;
      rel_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + rel_exp->getUniqueID() + "\";\n";
    }
    else if (type == Type::EQEXP)
    { // EqExp         ::= EqExp ("==" | "!=") RelExp;
      std::string node_def = node_id + "[label=\"<f0> EqExp | <f1> " + op + " | <f2> RelExp\"];\n";
      dot += node_def;
      eq_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + eq_exp->getUniqueID() + "\";\n";
      rel_exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + rel_exp->getUniqueID() + "\";\n";
    }
    else
    {
      std::cerr << "EqExpAST::toDot: unknown type" << std::endl;
    }
  }
};

class LAndExpAST : public BaseAST
{ // LAndExp       ::= EqExp | LAndExp "&&" EqExp;
public:
  enum class Type
  {
    EQEXP,
    LANDEXP
  } type;
  std::unique_ptr<BaseAST> eq_exp;
  std::unique_ptr<BaseAST> land_exp;
  std::string op;

  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::EQEXP)
    {
      return eq_exp->toIR(ir);
    }
    else
    { // 短路求值
      /* ret_value_t i1 = land_exp->toIR(ir);
      ret_value_t i2 = eq_exp->toIR(ir);
      // koopa IR 不支持 两个数直接与，需要判断两个数是否都不为0，然后按位与
      // ir += getIR("and", i1, i2);
      ir += getIR("ne", i1, {0, RetType::NUMBER});
      ir += getIR("ne", i2, {0, RetType::NUMBER});
      ir += getIR("and", {global_var_index - 2, RetType::INDEX}, {global_var_index - 1, RetType::INDEX});
      return {global_var_index - 1, RetType::INDEX}; */
      symbol_table.push(); // 为了防止result变量名重复，所以需要新建一个作用域
      // int result = 1;
      symbol_table.insert("result", {Item::Type::VAR, 1});
      ir += allocIR("result");
      ir += storeIR({0, RetType::NUMBER}, {RetValue("result"), RetType::IDENT});

      std::string if_label = "if_" + std::to_string(global_label_index);
      std::string end_label = "end_" + std::to_string(global_label_index++);

      ret_value_t i1 = land_exp->toIR(ir);
      ir += getIR("ne", i1, {0, RetType::NUMBER});
      ir += brIR({global_var_index - 1, RetType::INDEX}, if_label, end_label);

      ir += labelIR(if_label);
      ret_value_t i2 = eq_exp->toIR(ir);
      ir += getIR("ne", i2, {0, RetType::NUMBER});
      ir += storeIR({global_var_index - 1, RetType::INDEX}, {RetValue("result"), RetType::IDENT});
      ir += jumpIR(end_label);

      ir += labelIR(end_label);
      ir += loadIR({RetValue("result"), RetType::IDENT});
      symbol_table.pop();
      return {global_var_index - 1, RetType::INDEX};
    }
  }

  LAndExpAST(std::unique_ptr<BaseAST> &_eq_exp)
  {
    type = Type::EQEXP;
    eq_exp = std::move(_eq_exp);
  }
  LAndExpAST(std::unique_ptr<BaseAST> &_land_exp, std::string &_op, std::unique_ptr<BaseAST> &_eq_exp)
  {
    type = Type::LANDEXP;
    land_exp = std::move(_land_exp);
    op = _op;
    eq_exp = std::move(_eq_exp);
  }
  int calc() override
  {
    if (type == Type::EQEXP)
    {
      return eq_exp->calc();
    }
    else if (type == Type::LANDEXP)
    {
      int i1 = land_exp->calc();
      int i2 = eq_exp->calc();
      return i1 && i2;
    }
    else
    {
      std::cerr << "LAndExpAST::calc: unknown type" << std::endl;
      assert(0);
    }
    return 0;
  }
  void Dump() const override
  {
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

  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    if (type == Type::EQEXP)
    { // LAndExp       ::= EqExp;
      std::string node_def = node_id + "[label=\"<f0> EqExp\"];\n";
      dot += node_def;
      eq_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + eq_exp->getUniqueID() + "\";\n";
    }
    else if (type == Type::LANDEXP)
    { // LAndExp       ::= LAndExp "&&" EqExp;
      std::string node_def = node_id + "[label=\"<f0> LAndExp | <f1> && | <f2> EqExp\"];\n";
      dot += node_def;
      land_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + land_exp->getUniqueID() + "\";\n";
      eq_exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + eq_exp->getUniqueID() + "\";\n";
    }
    else
    {
      std::cerr << "LAndExpAST::toDot: unknown type" << std::endl;
    }
  }
};

class LOrExpAST : public BaseAST
{ // LOrExp        ::= LAndExp | LOrExp "||" LAndExp;
public:
  enum class Type
  {
    LANDEXP,
    LOREXP
  } type;
  std::unique_ptr<BaseAST> land_exp;
  std::unique_ptr<BaseAST> lor_exp;
  std::string op;

  LOrExpAST(std::unique_ptr<BaseAST> &_land_exp)
  {
    type = Type::LANDEXP;
    land_exp = std::move(_land_exp);
  }
  LOrExpAST(std::unique_ptr<BaseAST> &_lor_exp, std::string &_op, std::unique_ptr<BaseAST> &_land_exp)
  {
    type = Type::LOREXP;
    lor_exp = std::move(_lor_exp);
    op = _op;
    land_exp = std::move(_land_exp);
  }
  int calc() override
  {
    if (type == Type::LANDEXP)
    {
      return land_exp->calc();
    }
    else if (type == Type::LOREXP)
    {
      int i1 = lor_exp->calc();
      int i2 = land_exp->calc();
      return i1 || i2;
    }
    else
    {
      std::cerr << "LOrExpAST::calc: unknown type" << std::endl;
      assert(0);
    }
    return 0;
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::LANDEXP)
    {
      return land_exp->toIR(ir);
    }
    else if (type == Type::LOREXP)
    { // 短路求值
      /* ret_value_t i1 = lor_exp->toIR(ir);
      ret_value_t i2 = land_exp->toIR(ir);
      // koopa IR 不支持 两个数直接或，需要按位或，然后与0比较，得到结果
      // ir += getIR("or", i1, i2); 错误的
      ir += getIR("or", i1, i2);
      ir += getIR("ne", {global_var_index - 1, RetType::INDEX}, {0, RetType::NUMBER});
      return {global_var_index - 1, RetType::INDEX}; */
      symbol_table.push(); // 为了防止result变量名重复，所以需要新建一个作用域
      // int result = 1;
      symbol_table.insert("result", {Item::Type::VAR, 1});
      ir += allocIR("result");
      ir += storeIR({1, RetType::NUMBER}, {RetValue("result"), RetType::IDENT});

      std::string if_label = "if_" + std::to_string(global_label_index);
      std::string end_label = "end_" + std::to_string(global_label_index++);

      ret_value_t i1 = lor_exp->toIR(ir);
      ir += getIR("eq", i1, {0, RetType::NUMBER});
      ir += brIR({global_var_index - 1, RetType::INDEX}, if_label, end_label);

      ir += labelIR(if_label);
      ret_value_t i2 = land_exp->toIR(ir);
      ir += getIR("ne", i2, {0, RetType::NUMBER});
      ir += storeIR({global_var_index - 1, RetType::INDEX}, {RetValue("result"), RetType::IDENT});
      ir += jumpIR(end_label);

      ir += labelIR(end_label);
      ir += loadIR({RetValue{"result"}, RetType::IDENT});
      symbol_table.pop();
      return {global_var_index - 1, RetType::INDEX};
    }
    else
    {
      std::cerr << "LOrExpAST::toIR: unknown type" << std::endl;
      assert(0);
    }
    return {1, RetType::VOID};
  }

  void Dump() const override
  {
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

  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    if (type == Type::LANDEXP)
    { // LOrExp       ::= LAndExp;
      std::string node_def = node_id + "[label=\"<f0> LAndExp\"];\n";
      dot += node_def;
      land_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + land_exp->getUniqueID() + "\";\n";
    }
    else if (type == Type::LOREXP)
    { // LOrExp       ::= LOrExp "||" LAndExp;
      std::string node_def = node_id + "[label=\"<f0> LOrExp | <f1> || | <f2> LAndExp\"];\n";
      dot += node_def;
      lor_exp->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + lor_exp->getUniqueID() + "\";\n";
      land_exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + land_exp->getUniqueID() + "\";\n";
    }
    else
    {
      std::cerr << "LOrExpAST::toDot: unknown type" << std::endl;
    }
  }
};

class ConstExpAST : public BaseAST
{ // ConstExp      ::= Exp;
public:
  std::unique_ptr<BaseAST> exp;
  ConstExpAST(std::unique_ptr<BaseAST> &_exp) : exp(std::move(_exp)) {}
  void Dump() const override
  {
    std::cout << "ConstExpAST { ";
    exp->Dump();
    std::cout << " }";
  }
  int calc() override
  {
    return exp->calc();
  }
  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"<f0> Exp\"];\n";
    dot += node_def;
    exp->toDot(dot);
    dot += "\"" + node_id + "\":f0 ->" + "\"" + exp->getUniqueID() + "\";\n";
  }
};
