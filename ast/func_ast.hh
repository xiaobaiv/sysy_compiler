#pragma once
#include "base_ast.hh"

class FuncDefAST : public BaseAST
{ // FuncDef       ::= FuncType IDENT "(" [FuncFParams] ")" Block;
public:
  enum class Option
  {
    F0,
    F1
  } option;
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> func_fparams;
  std::unique_ptr<BaseAST> block;

  /* ir 语法
  FunDef ::= "fun" SYMBOL "(" [FunParams] ")" [":" Type] "{" FunBody "}";
  FunParams ::= SYMBOL ":" Type {"," SYMBOL ":" Type};
  FunBody ::= {Block};
  Block ::= SYMBOL ":" {Statement} EndStatement;
  Statement ::= SymbolDef | Store | FunC;
  EndStatement ::= Branch | Jump | Return;
   */

  // 生成ir 的函数 toIR, 结果存在 字符串 ir 中，遵循ir 语法
  ret_value_t toIR(std::string &ir) override
  {
    // 添加符号表
    if (!symbol_table.insert(ident, {Item::Type::FUNC, func_type->isVoid() ? 0 : 1}))
    { // 检查全局变量或函数是否重名
      std::cerr << "FuncDefAST::toIR: function name " << ident << " already exists" << std::endl;
      assert(0);
    }
    symbol_table.push();

    if (option == Option::F0)
    { // FuncDef       ::= FuncType IDENT "(" ")" Block; 遵循ir语法，生成ir语言FunDef ::= "fun" SYMBOL "(" [FunParams] ")" [":" Type] "{" FunBody "}";
      ir += "fun @" + ident + "()";
      func_type->toIR(ir);
      ir += " {\n";
      ir += "%entry:\n"; // %entry 与 函数定义关联
      block->toIR(ir);
    }
    else if (option == Option::F1)
    { // FuncDef       ::= FuncType IDENT "(" FuncFParams ")" Block; 遵循ir语法，生成ir语言FunDef ::= "fun" SYMBOL "(" FunParams ")" [":" Type] "{" FunBody "}";
      ir += "fun @" + ident + "(";
      func_fparams->toIR(ir); // 需要将参数放到子作用域中 todo
      ir += ")";
      func_type->toIR(ir);
      ir += " {\n";
      ir += "%entry:\n"; // %entry 与 函数定义关联
      func_fparams->xx(ir);
      block->toIR(ir);
    }
    else
    {
      std::cerr << "FuncDefAST::toIR: unknown option" << std::endl;
    }

    // 判断函数是否有返回值
    if (!isEnd(ir, 1))
    {
      // 如果函数类型是void，则不需要返回值
      if (func_type->isVoid())
      {
        ir += "\tret\n";
      }
      else
      {
        ir += "\tret 0\n";
      }
    }
    // 解析ir，最后的ret指令，查看是否有返回值
    if (!isEnd(ir, 2) && !func_type->isVoid())
    {
      std::cerr << "FuncDefAST::toIR: function " << ident << " has no return value" << std::endl;
      assert(0);
    }
    ir += "}\n";
    // 删除符号表
    symbol_table.pop();
    return {0, RetType::VOID};
  }

  void Dump() const override
  {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    if (option == Option::F1)
    {
      func_fparams->Dump();
    }
    block->Dump();
    std::cout << " }";
  }

  void toDot(std::string &dot) const override
  { // FuncDef       ::= FuncType IDENT "(" [FuncFParams] ")" Block; 需要显示终结符，并单独设为一个field, 就像LValAST那样
    std::string node_id = getUniqueID();
    if (option == Option::F0)
    { // FuncDef       ::= FuncType IDENT "(" ")" Block;
      std::string node_def = node_id + "[label=\"<f0> FuncType | <f1> IDENT: " + ident + " | <f2> ( | <f3> ) | <f4> Block\"];\n";
      dot += node_def;
      func_type->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + func_type->getUniqueID() + "\";\n";
      block->toDot(dot);
      dot += "\"" + node_id + "\":f4 ->" + "\"" + block->getUniqueID() + "\";\n";
    }
    else if (option == Option::F1)
    { // FuncDef       ::= FuncType IDENT "(" FuncFParams ")" Block;
      std::string node_def = node_id + "[label=\"<f0> FuncType | <f1> IDENT: " + ident + " | <f2> ( | <f3> FuncFParams | <f4> ) | <f5> Block\"];\n";
      dot += node_def;
      func_type->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + func_type->getUniqueID() + "\";\n";
      func_fparams->toDot(dot);
      dot += "\"" + node_id + "\":f3 ->" + "\"" + func_fparams->getUniqueID() + "\";\n";
      block->toDot(dot);
      dot += "\"" + node_id + "\":f5 ->" + "\"" + block->getUniqueID() + "\";\n";
    }
    else
    {
      std::cerr << "FuncDefAST::toDot: unknown option" << std::endl;
    }
  }
};

class FuncTypeAST : public BaseAST
{ // FuncType      ::= "void" | "int";
public:
  std::string type;

  bool isVoid() override
  {
    return type == "void";
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (type == "int")
    {
      ir += ": i32";
    }
    else if (type == "void")
    {
      ir += "";
    }
    else
    {
      std::cerr << "FuncTypeAST::toIR: unknown type" << std::endl;
    }
    return {0, RetType::VOID};
  }
  void Dump() const override
  {
    std::cout << "FuncTypeAST { ";
    std::cout << type << " }";
  }

  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    std::string node_label = "FuncType: " + type; // 直接将类型名称加入标签
    std::string node_def = node_id + " [label=\"" + node_label + "\"];\n";
    dot += node_def;
  }
};

class FuncFParamsAST : public BaseAST
{ // FuncFParams   ::= FuncFParam {"," FuncFParam};
public:
  List fparams_list;
  FuncFParamsAST(List &_fparams_list)
  {
    for (auto &item : _fparams_list)
    {
      fparams_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
  }

  ret_value_t toIR(std::string &ir) override
  {
    for (int i = 0; i < fparams_list.size(); i++)
    {
      fparams_list[i].second->toIR(ir);
      if (i != fparams_list.size() - 1)
      {
        ir += ", ";
      }
    }
    return {0, RetType::VOID};
  }

  void xx(std::string &ir) override
  {
    for (auto &item : fparams_list)
    {
      item.second->xx(ir);
    }
  }

  void Dump() const override
  {
    std::cout << "FuncFParamsAST { ";
    for (auto &item : fparams_list)
    {
      item.second->Dump();
    }
    std::cout << " }";
  }
  void toDot(std::string &dot) const override
  { // FuncFParams   ::= FuncFParam {"," FuncFParam};
    std::string node_id = getUniqueID();
    std::string node_def = node_id + "[label=\"<f0> FuncFParam";
    for (int i = 1; i < fparams_list.size(); i++)
    {
      node_def += " | <f" + std::to_string(i * 2 - 1) + "> , | <f" + std::to_string(i * 2) + "> FuncFParam";
    }
    node_def += "\"];\n";
    dot += node_def;

    for (int i = 0; i < fparams_list.size(); i++)
    {
      fparams_list[i].second->toDot(dot);
      dot += "\"" + node_id + "\":f" + std::to_string(i * 2) + " ->" + "\"" + fparams_list[i].second->getUniqueID() + "\";\n";
    }
  }
};

class FuncFParamAST : public BaseAST
{ // FuncFParam    ::= BType IDENT ["[" "]" {"[" ConstExp "]"}];
public:
  enum class Option
  {
    C0,
    C1
  } option;
  std::string ident;
  List const_exp_list;
  std::unique_ptr<BaseAST> btype;
  FuncFParamAST(Option _option, const std::string &_ident, List &_const_exp_list, std::unique_ptr<BaseAST> &_btype) : option(_option), ident(_ident), btype(std::move(_btype))
  {
    for (auto &item : _const_exp_list)
    {
      const_exp_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
  }
  ret_value_t toIR(std::string &ir) override
  {
    if (option == Option::C0)
    { // FuncFParam    ::= BType IDENT ;
      if (!symbol_table.insert("param_" + ident, {Item::Type::VAR, 0}))
      { // 检查全局变量或函数是否重名
        std::cerr << "FuncFParamAST::toIR: variable name " << ident << " already exists" << std::endl;
        assert(0);
      }
      ir += "@" + symbol_table.getUniqueIdent("param_" + ident) + ": i32";
    }
    else if (option == Option::C1)
    { // FuncFParam    ::= BType IDENT "[" "]" {"[" ConstExp "]"};
      // 数组参数，实际上是一个指针，需要在函数内部分配内存。例如：int f(int arr[]) { return arr[1]; } 生成的ir代码如下
      /*  fun @f(@arr: *i32): i32 {
          %entry:
            %arr = alloc *i32
            store @arr, %arr
            %0 = load %arr
            %1 = getptr %0, 1
            %2 = load %1
            ret %2
          } */
      if (!symbol_table.insert("param_" + ident, {Item::Type::PTR, 0})) // 检查全局变量或函数是否重名
      {
        std::cerr << "FuncFParamAST::toIR: variable (ptr)name " << ident << " already exists" << std::endl;
        assert(0);
      }
      std::vector<int> dims;
      for (auto &item : const_exp_list)
      {
        dims.push_back(item.second->calc());
      }
      // 生成参数的ir代码
      // 解析dims
      ir += "@" + symbol_table.getUniqueIdent("param_" + ident) + ": ";
      ir += genDim(dims);
    }
    else
    {
      std::cerr << "FuncFParamAST::toIR: unknown option" << std::endl;
    }
    return {0, RetType::VOID};
  }
  std::string genDim(std::vector<int> &size)
  {
    std::string ir = "*";
    // 生成数组的维度部分
    if (size.size() == 0)
    {
      return "*i32";
    }
    for (size_t i = 0; i < size.size(); ++i)
    {
      ir += "["; // 开始一层新的维度
    }

    // 添加最内层类型
    ir += "i32";

    // 从内向外填充维度大小
    for (auto it = size.rbegin(); it != size.rend(); ++it)
    {
      ir += ", " + std::to_string(*it) + "]";
    }
    return ir;
  }
  // 在块中将形参存入内存
  void xx(std::string &ir) override
  {
    if (option == Option::C0)
    { // FuncFParam    ::= BType IDENT ;
      if (!symbol_table.insert(ident, {Item::Type::VAR, 0}))
      { // 检查全局变量或函数是否重名
        std::cerr << "FuncFParamAST::toIR: variable name: " << ident << " already exists" << std::endl;
        assert(0);
      }
      ir += allocIR(ident);
      ir += storeIR({"param_" + ident, RetType::IDENT}, {ident, RetType::IDENT});
    }
    else if (option == Option::C1)
    { // FuncFParam    ::= BType IDENT "[" "]" {"[" ConstExp "]"};
      if (!symbol_table.insert(ident, {Item::Type::PTR, static_cast<int>(const_exp_list.size() + 1)}))
      { // 检查全局变量或函数是否重名
        std::cerr << "FuncFParamAST::toIR: variable name: " << ident << " already exists" << std::endl;
        assert(0);
      }
      std::vector<int> dims;
      for (auto &item : const_exp_list)
      {
        dims.push_back(item.second->calc());
      }
      ir += allocIR(ident, genDim(dims));
      ir += storeIR({"param_" + ident, RetType::IDENT}, {ident, RetType::IDENT});
    }
    else
    {
      std::cerr << "FuncFParamAST::toIR: unknown option" << std::endl;
    }
  }

  void Dump() const override
  {
    std::cout << "FuncFParamAST { " << ident;
    for (auto &item : const_exp_list)
    {
      std::cout << "[";
      item.second->Dump();
      std::cout << "]";
    }
    std::cout << " }";
  }

  void toDot(std::string &dot) const override
  { // FuncFParam    ::= BType IDENT ["[" "]" {"[" ConstExp "]"}]; 需要显示终结符，并单独设为一个field, 就像LValAST那样
    std::string node_id = getUniqueID();
    if (option == Option::C0)
    { // FuncFParam    ::= BType IDENT ;
      std::string node_def = node_id + "[label=\"<f0> BType | <f1> IDENT: " + ident + "\"];\n";
      dot += node_def;
      btype->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + btype->getUniqueID() + "\";\n";
    }
    else if (option == Option::C1)
    { // FuncFParam    ::= BType IDENT "[" "]" {"[" ConstExp "]"};
      std::string node_def = node_id + "[label=\"<f0> BType | <f1> IDENT: " + ident + " | <f2> [ | <f3> ]";
      for (int i = 0; i < const_exp_list.size(); i++)
      {
        node_def += " | <f" + std::to_string(i * 3 + 4) + "> [ | <f" + std::to_string(i * 3 + 5) + "> ConstExp | <f" + std::to_string(i * 3 + 6) + "> ]";
      }
      node_def += "\"];\n";
      dot += node_def;

      btype->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + btype->getUniqueID() + "\";\n";

      for (int i = 0; i < const_exp_list.size(); i++)
      {
        const_exp_list[i].second->toDot(dot);
        dot += "\"" + node_id + "\":f" + std::to_string(i * 3 + 5) + " ->" + "\"" + const_exp_list[i].second->getUniqueID() + "\";\n";
      }
    }
    else
    {
      std::cerr << "FuncFParamAST::toDot: unknown option" << std::endl;
    }
  }
};

class BlockAST : public BaseAST
{ // Block         ::= "{" {BlockItem} "}";
public:
  List block_item_list;
  BlockAST() {}
  BlockAST(List &_block_item_list)
  {
    for (auto &item : _block_item_list)
    {
      block_item_list.push_back(std::make_pair(item.first, std::move(item.second)));
    }
  }

  ret_value_t toIR(std::string &ir) override
  { // %entry:

    for (auto &item : block_item_list)
    {
      item.second->toIR(ir);
    }
    return {0, RetType::VOID};
  }

  void Dump() const override
  {
    std::cout << "BlockAST { ";
    for (auto &item : block_item_list)
    {
      item.second->Dump();
    }
    std::cout << " }";
  }

  void toDot(std::string &dot) const override
  { // Block         ::= "{" {BlockItem} "}";
    std::string node_id = getUniqueID();
    if (block_item_list.size() == 0)
    { // Block         ::= "{" "}";
      std::string node_def = node_id + "[label=\"<f0> \\{ |<f1> \\}" + "\"];\n";
      dot += node_def;
    }
    else
    { // Block         ::= "{" {BlockItem} "}";
      std::string node_def = node_id + "[label=\"<f0> \\{";
      for (int i = 0; i < block_item_list.size(); i++)
      {
        node_def += " | <f" + std::to_string(i + 1) + "> BlockItem";
      }
      node_def += " | <f" + std::to_string(block_item_list.size() + 1) + "> \\}" + "\"];\n";
      dot += node_def;

      for (int i = 0; i < block_item_list.size(); i++)
      {
        block_item_list[i].second->toDot(dot);
        dot += "\"" + node_id + "\":f" + std::to_string(i + 1) + " ->" + "\"" + block_item_list[i].second->getUniqueID() + "\";\n";
      }
    }
  }
};

class BlockItemAST : public BaseAST
{ // BlockItem     ::= Decl | Stmt;
public:
  std::unique_ptr<BaseAST> decl_or_stmt;
  enum class Type
  {
    DECL,
    STMT
  } type;

  BlockItemAST(std::unique_ptr<BaseAST> &_decl_or_stmt, Type _type) : decl_or_stmt(std::move(_decl_or_stmt)), type(_type) {}

  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::DECL)
    { // BlockItem     ::= Decl;
      decl_or_stmt->toIR(ir);
    }
    else if (type == Type::STMT)
    { // BlockItem     ::= Stmt;
      decl_or_stmt->toIR(ir);
    }
    else
    {
      std::cerr << "BlockItemAST::toIR: unknown type" << std::endl;
    }
    return {0, RetType::VOID};
  }

  void Dump() const override
  {
    std::cout << "BlockItemAST { ";
    switch (type)
    {
    case Type::DECL:
      decl_or_stmt->Dump();
      break;
    case Type::STMT:
      decl_or_stmt->Dump();
      break;
    }
    std::cout << " }";
  }

  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    if (type == Type::DECL)
    { // BlockItem     ::= Decl;
      std::string node_def = node_id + "[label=\"<f0> Decl\"];\n";
      dot += node_def;
      decl_or_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + decl_or_stmt->getUniqueID() + "\";\n";
    }
    else if (type == Type::STMT)
    { // BlockItem     ::= Stmt;
      std::string node_def = node_id + "[label=\"<f0> Stmt\"];\n";
      dot += node_def;
      decl_or_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + decl_or_stmt->getUniqueID() + "\";\n";
    }
    else
    {
      std::cerr << "BlockItemAST::toDot: unknown type" << std::endl;
    }
  }
};

class StmtAST : public BaseAST
{
public:
  enum class Type
  {
    ASSIGN,
    RETURN,
    BLOCK,
    EXP,
    IF,
    IFELSE,
    WHILE,
    BREAK,
    CONTINUE
  } type;
  enum class Option
  {
    EXP0,
    EXP1
  } option;
  std::unique_ptr<BaseAST> lval;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> if_stmt;
  std::unique_ptr<BaseAST> else_stmt;
  StmtAST(std::unique_ptr<BaseAST> &_lval, std::unique_ptr<BaseAST> &_exp) : lval(std::move(_lval)), exp(std::move(_exp))
  {
    type = Type::ASSIGN;
  }
  StmtAST() {}
  /* Stmt ::=  LVal "=" Exp ";"
              | [Exp] ";"
              | Block
              | "if" "(" Exp ")" Stmt ["else" Stmt]
              | "while" "(" Exp ")" Stmt
              | "break" ";"
              | "continue" ";"
              | "return" [Exp] ";";
  */
  ret_value_t toIR(std::string &ir) override
  {
    if (type == Type::RETURN && !isEnd(ir))
    {

      if (option == Option::EXP0)
      { // Stmt          ::= "return" ";";
        ir += "\tret\n";
      }
      else if (option == Option::EXP1)
      { // Stmt          ::= "return" Exp ";";
        ret_value_t ret = exp->toIR(ir);
        if (ret.second == RetType::NUMBER)
        {
          ir += "\tret " + std::to_string(ret.first.number) + "\n";
        }
        else if (ret.second == RetType::INDEX)
        {
          ir += "\tret %" + std::to_string(ret.first.number) + "\n";
        }
        else if (ret.second == RetType::VOID)
        {
          ir += "\tret\n";
        }
        else if (ret.second == RetType::IDENT)
        {
          ir += loadIR(ret);
          ir += "\tret %" + std::to_string(global_var_index - 1) + "\n";
        }
        else if (ret.second == RetType::ARRAYPTR)
        {
          ir += "\tret %ptr" + std::to_string(ret.first.number) + "\n";
        }
        else
        {
          std::cerr << "StmtAST::toIR: unknown ret type" << std::endl;
        }
      }
    }
    else if (type == Type::ASSIGN)
    {
      ret_value_t exp_ret = exp->toIR(ir);
      ret_value_t lval_ret = lval->toIR(ir);
      ir += storeIR(exp_ret, lval_ret);
    }
    else if (type == Type::EXP)
    {
      if (option == Option::EXP1)
      {
        exp->toIR(ir);
      }
    }
    else if (type == Type::BLOCK)
    {
      symbol_table.push();
      block->toIR(ir);
      symbol_table.pop();
    }
    else if (type == Type::IF)
    { // "if" "(" Exp ")" Stmt
      std::string then_label = "then_" + std::to_string(global_label_index);
      std::string end_label = "end_" + std::to_string(global_label_index++);
      ir += "\t// if 的条件判断部分\n";
      ret_value_t exp_ret = exp->toIR(ir);
      ir += brIR(exp_ret, then_label, end_label);
      ir += "\n// if 语句的 if 分支 \n";
      ir += labelIR(then_label);
      if_stmt->toIR(ir);
      ir += jumpIR(end_label);
      ir += "\n// if 语句之后的内容, if/else 分支的交汇处 \n";
      ir += labelIR(end_label);
    }
    else if (type == Type::IFELSE)
    { // "if" "(" Exp ")" Stmt "else" Stmt
      std::string then_label = "then_" + std::to_string(global_label_index);
      std::string else_label = "else_" + std::to_string(global_label_index);
      std::string end_label = "end_" + std::to_string(global_label_index++);
      ir += "\t// if 的条件判断部分\n";
      ret_value_t exp_ret = exp->toIR(ir);
      ir += brIR(exp_ret, then_label, else_label);
      ir += "\n// if 语句的 if 分支 \n";
      ir += labelIR(then_label);
      if_stmt->toIR(ir);
      ir += jumpIR(end_label);
      ir += "\n// if 语句的 else 分支 \n";
      ir += labelIR(else_label);
      else_stmt->toIR(ir);
      ir += jumpIR(end_label);
      ir += "\n// if 语句之后的内容, if/else 分支的交汇处 \n";
      ir += labelIR(end_label);
    }
    else if (type == Type::WHILE)
    { // "while" "(" Exp ")" Stmt

      loop_stack.push(global_label_index); // 压入循环的标签号
      std::string while_entry_label = "while_entry_" + std::to_string(global_label_index);
      std::string while_body_label = "while_body_" + std::to_string(global_label_index);
      std::string end_label = "end_" + std::to_string(global_label_index++);

      ir += jumpIR(while_entry_label);
      ir += "\t// while 循环的入口\n";
      ir += labelIR(while_entry_label);
      ret_value_t exp_ret = exp->toIR(ir);
      ir += brIR(exp_ret, while_body_label, end_label);

      ir += "\n// while 循环的主体\n";
      ir += labelIR(while_body_label);
      if_stmt->toIR(ir);
      ir += jumpIR(while_entry_label);

      ir += "\n// while 循环结束\n";
      ir += labelIR(end_label);
      loop_stack.pop(); // 弹出循环的标签号
    }
    else if (type == Type::BREAK)
    { // "break" ";";
      // 判断是否在循环中
      if (loop_stack.empty())
      {
        std::cerr << "break 语句不在循环中" << std::endl;
        assert(0);
      }
      std::string end_label = "end_" + std::to_string(loop_stack.top());
      ir += jumpIR(end_label);
    }
    else if (type == Type::CONTINUE)
    { // "continue" ";";
      // 判断是否在循环中
      if (loop_stack.empty())
      {
        std::cerr << "continue 语句不在循环中" << std::endl;
        assert(0);
      }
      std::string while_entry_label = "while_entry_" + std::to_string(loop_stack.top());
      ir += jumpIR(while_entry_label);
    }
    else
    {
      std::cerr << "StmtAST::toIR: unknown type" << std::endl;
    }

    return {0, RetType::VOID};
  }

  void Dump() const override
  {
    std::cout << "StmtAST { ";
    switch (type)
    {
    case Type::ASSIGN:
      lval->Dump();
      std::cout << " = ";
      exp->Dump();
      break;
    case Type::RETURN:
      std::cout << "return ";
      if (option == Option::EXP1)
      {
        exp->Dump();
      }
      break;
    case Type::BLOCK:
      block->Dump();
      break;
    case Type::EXP:
      if (option == Option::EXP1)
      {
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

  void toDot(std::string &dot) const override
  {
    std::string node_id = getUniqueID();
    if (type == Type::ASSIGN)
    { // Stmt          ::= LVal "=" Exp ";"
      std::string node_def = node_id + "[label=\"<f0> LVal | <f1> = | <f2> Exp\"];\n";
      dot += node_def;
      lval->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + lval->getUniqueID() + "\";\n";
      exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + exp->getUniqueID() + "\";\n";
    }
    else if (type == Type::RETURN)
    { // Stmt          ::= "return" [Exp] ";";
      if (option == Option::EXP0)
      { // Stmt          ::= "return" ";";
        std::string node_def = node_id + "[label=\"<f0> return | <f1> ;\"];\n";
        dot += node_def;
      }
      else if (option == Option::EXP1)
      { // Stmt          ::= "return" Exp ";";
        std::string node_def = node_id + "[label=\"<f0> return | <f1> Exp | <f2> ;\"];\n";
        dot += node_def;
        exp->toDot(dot);
        dot += "\"" + node_id + "\":f1 ->" + "\"" + exp->getUniqueID() + "\";\n";
      }
      else
      {
        std::cerr << "StmtAST::toDot: unknown option" << std::endl;
      }
    }
    else if (type == Type::BLOCK)
    { // Stmt          ::= Block;
      std::string node_def = node_id + "[label=\"<f0> Block\"];\n";
      dot += node_def;
      block->toDot(dot);
      dot += "\"" + node_id + "\":f0 ->" + "\"" + block->getUniqueID() + "\";\n";
    }
    else if (type == Type::EXP)
    { // Stmt          ::= [Exp] ";";
      if (option == Option::EXP0)
      { // Stmt          ::= ";";
        std::string node_def = node_id + "[label=\"<f0> ;\"];\n";
        dot += node_def;
      }
      else if (option == Option::EXP1)
      { // Stmt          ::= Exp ";";
        std::string node_def = node_id + "[label=\"<f0> Exp | <f1> ;\"];\n";
        dot += node_def;
        exp->toDot(dot);
        dot += "\"" + node_id + "\":f0 ->" + "\"" + exp->getUniqueID() + "\";\n";
      }
      else
      {
        std::cerr << "StmtAST::toDot: unknown option" << std::endl;
      }
    }
    else if (type == Type::IF)
    { // Stmt          ::= "if" "(" Exp ")" Stmt;
      std::string node_def = node_id + "[label=\"<f0> if | <f1> ( | <f2> Exp | <f3> ) | <f4> Stmt\"];\n";
      dot += node_def;
      exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + exp->getUniqueID() + "\";\n";
      if_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f4 ->" + "\"" + if_stmt->getUniqueID() + "\";\n";
    }
    else if (type == Type::IFELSE)
    { // Stmt          ::= "if" "(" Exp ")" Stmt "else" Stmt;
      std::string node_def = node_id + "[label=\"<f0> if | <f1> ( | <f2> Exp | <f3> ) | <f4> Stmt | <f5> else | <f6> Stmt\"];\n";
      dot += node_def;
      exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + exp->getUniqueID() + "\";\n";
      if_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f4 ->" + "\"" + if_stmt->getUniqueID() + "\";\n";
      else_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f6 ->" + "\"" + else_stmt->getUniqueID() + "\";\n";
    }
    else if (type == Type::WHILE)
    { // Stmt          ::= "while" "(" Exp ")" Stmt;
      std::string node_def = node_id + "[label=\"<f0> while | <f1> ( | <f2> Exp | <f3> ) | <f4> Stmt\"];\n";
      dot += node_def;
      exp->toDot(dot);
      dot += "\"" + node_id + "\":f2 ->" + "\"" + exp->getUniqueID() + "\";\n";
      if_stmt->toDot(dot);
      dot += "\"" + node_id + "\":f4 ->" + "\"" + if_stmt->getUniqueID() + "\";\n";
    }
    else if (type == Type::BREAK)
    { // Stmt          ::= "break" ";";
      std::string node_def = node_id + "[label=\"<f0> break | <f1> ;\"];\n";
      dot += node_def;
    }
    else if (type == Type::CONTINUE)
    { // Stmt          ::= "continue" ";";
      std::string node_def = node_id + "[label=\"<f0> continue | <f1> ;\"];\n";
      dot += node_def;
    }
    else
    {
      std::cerr << "StmtAST::toDot: unknown type" << std::endl;
    }
  }
};
