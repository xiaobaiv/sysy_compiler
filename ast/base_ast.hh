#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <stack>
#include <assert.h>
#include <unordered_map>
#include <algorithm>
#include "sysbol_table.hh"
#include <stdarg.h>
#include <functional>

enum class RetType
{
  NUMBER,
  INDEX,
  VOID,
  IDENT,
  PTR,
  ARRAY,
  ARRAYPTR,
  ELEMENTPTR
};
struct RetValue
{
  int number;
  std::string ident;
  RetValue(int n) : number(n) {}
  RetValue(std::string id) : ident(id) {}
  RetValue() {}
};

typedef std::pair<RetValue, RetType> ret_value_t;
typedef uint64_t var_index_t;
enum class ListType
{
  CONSTDEF,
  VARDEF,
  DECL,
  STMT,
  BLOCKITEM,
  FUNCFPARAM,
  EXP,
  CONSTEXP,
  INITVAL,
  CONSTINITVAL
};
// 所有 AST 的基类
class BaseAST
{
public:
  static var_index_t global_var_index;
  static var_index_t global_label_index;
  static var_index_t global_ptr_index;
  // 符号表
  // static std::unordered_map<std::string, int> symbol_table;
  static SymbolTable symbol_table;
  static std::string ir;
  static std::stack<var_index_t> loop_stack;

public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
  virtual bool isArray()
  {
    std::cerr << "isArray not implemented\n";
    assert(0);
  }
  virtual void toDot(std::string &dot) const = 0;
  virtual int calc()
  {
    std::cerr << "calc not implemented\n";
    assert(0);
  }
  virtual void calc(std::vector<int> &init_list, int start_index, int end_index, std::vector<int> &array_size, int sub_array_size)
  {
    std::cerr << "calc not implemented\n";
    assert(0);
  }
  virtual ret_value_t toIR(std::string &ir)
  {
    std::cerr << "toIR not implemented\n\n"
              << ir << "\n";
    assert(0);
  }

  virtual void xx(std::string &ir)
  {
    std::cerr << "xx not implemented\n";
    assert(0);
  }
  virtual void readArgs(std::vector<ret_value_t> &args)
  {
    std::cerr << "readArgs not implemented\n";
    assert(0);
  }
  virtual bool isVoid()
  {
    std::cerr << "isVoid not implemented\n";
    assert(0);
  }
  // 为了区分, 我们为每个 AST 节点(label)生成一个唯一的 ID
  virtual std::string getUniqueID() const
  {
    return "Node_" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
  }
  virtual std::string getIR(std::string op, ret_value_t ret1, ret_value_t ret2) const
  {
    if (isEnd(ir))
    {
      return "";
    }
    std::string ir;
    if (ret1.second == RetType::NUMBER)
    {
      if (ret2.second == RetType::NUMBER)
      {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " " + std::to_string(ret1.first.number) + ", " + std::to_string(ret2.first.number) + "\n";
      }
      else if (ret2.second == RetType::INDEX)
      {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " " + std::to_string(ret1.first.number) + ", %" + std::to_string(ret2.first.number) + "\n";
      }
      else
      {
        std::cerr << "getIR: unknown ret2 type\n";
      }
    }
    else if (ret1.second == RetType::INDEX)
    {
      if (ret2.second == RetType::NUMBER)
      {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " %" + std::to_string(ret1.first.number) + ", " + std::to_string(ret2.first.number) + "\n";
      }
      else if (ret2.second == RetType::INDEX)
      {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " %" + std::to_string(ret1.first.number) + ", %" + std::to_string(ret2.first.number) + "\n";
      }
      else
      {
        std::cerr << "getIR: unknown ret2 type\n";
      }
    }
    else
    {
      std::cerr << "getIR: unknown ret1 type\n";
    }
    return ir;
  }
  virtual std::string storeIR(ret_value_t ret1, ret_value_t ret2) const
  {
    if (isEnd(ir))
    {
      return "";
    }
    std::string ir;
    // ret2.second must be INDENT or PTR
    if (ret1.second == RetType::NUMBER)
    {
      if (ret2.second == RetType::IDENT)
        ir = "\tstore " + std::to_string(ret1.first.number) + ", @" + symbol_table.getUniqueIdent(ret2.first.ident) + "\n";
      else
        ir = "\tstore " + std::to_string(ret1.first.number) + ", %ptr" + std::to_string(ret2.first.number) + "\n";
    }
    else if (ret1.second == RetType::INDEX)
    {
      if (ret2.second == RetType::IDENT)
        ir = "\tstore %" + std::to_string(ret1.first.number) + ", @" + symbol_table.getUniqueIdent(ret2.first.ident) + "\n";
      else
        ir = "\tstore %" + std::to_string(ret1.first.number) + ", %ptr" + std::to_string(ret2.first.number) + "\n";
    }
    else if (ret1.second == RetType::IDENT)
    {
      if (ret2.second == RetType::IDENT)
        ir = "\tstore @" + symbol_table.getUniqueIdent(ret1.first.ident) + ", @" + symbol_table.getUniqueIdent(ret2.first.ident) + "\n";
      else
        ir = "\tstore @" + symbol_table.getUniqueIdent(ret1.first.ident) + ", %ptr" + std::to_string(ret2.first.number) + "\n";
    }
    else
    {
      std::cerr << "storeIR: unknown ret1 type\n";
      assert(0);
    }
    return ir;
  }
  virtual std::string loadIR(ret_value_t ret) const
  {
    if (isEnd(ir))
    {
      return "";
    }
    std::string ir;
    if (ret.second == RetType::IDENT)
    {
      ir = "\t%" + std::to_string(global_var_index++) + " = load @" + symbol_table.getUniqueIdent(ret.first.ident) + "\n";
    }
    else if (ret.second == RetType::PTR)
    {
      ir = "\t%" + std::to_string(global_var_index++) + " = load %ptr" + std::to_string(ret.first.number) + "\n";
    }
    else if (ret.second == RetType::ARRAYPTR)
    {
      ir += "\t%ptr" + std::to_string(global_ptr_index++) + " = load @" + symbol_table.getUniqueIdent(ret.first.ident) + "\n";
    }
    else if (ret.second == RetType::ELEMENTPTR)
    {
      ir += "\t%" + std::to_string(global_var_index++) + " = load %ptr" + std::to_string(ret.first.number) + "\n";
    }
    else
    {
      std::cerr << "loadIR: unknown ret type\n";
      assert(0);
    }
    return ir;
  }

  virtual std::string brIR(ret_value_t ret, std::string label1, std::string label2) const
  {
    if (isEnd(ir))
    {
      return "";
    }
    std::string ir;
    if (ret.second == RetType::NUMBER)
    {
      ir = "\tbr " + std::to_string(ret.first.number) + ", %" + label1 + ", %" + label2 + "\n";
    }
    else if (ret.second == RetType::INDEX)
    {
      ir = "\tbr %" + std::to_string(ret.first.number) + ", %" + label1 + ", %" + label2 + "\n";
    }
    else
    {
      std::cerr << "brIR: unknown ret type\n";
      assert(0);
    }
    return ir;
  }
  virtual std::string labelIR(std::string label) const
  {
    return "%" + label + ":\n";
  }
  virtual std::string jumpIR(std::string label) const
  {
    if (isEnd(ir))
    {
      return "";
    }
    return "\tjump %" + label + "\n";
  }

  virtual std::string get_last_instruction(std::string &ir) const
  {
    // 从最后一个换行符开始向前找倒数第二个换行符
    size_t last_newline = ir.find_last_of('\n');
    if (last_newline == std::string::npos || last_newline == 0)
    {
      // 如果没有找到换行符或者只有一个换行符，则说明没有有效的指令
      return "";
    }

    size_t second_last_newline = ir.find_last_of('\n', last_newline - 1);
    if (second_last_newline == std::string::npos)
    {
      // 如果找不到倒数第二个换行符，说明只有一行指令
      second_last_newline = 0;
    }
    else
    {
      // 否则将第二个换行符位置移到下一个字符，开始提取最后一条指令
      second_last_newline += 1;
    }

    // 提取最后一条指令
    std::string last_instruction = ir.substr(second_last_newline, last_newline - second_last_newline);
    last_instruction.erase(std::remove_if(last_instruction.begin(), last_instruction.end(), ::isspace), last_instruction.end());
    return last_instruction;
  }

  // 为了防止在生成 IR 时在终止指令（ret、br、jump）之后还有其他指令，我们需要从ir中提取出最后一条指令，当前ir的最后一个字符一定是换行符(即倒数第二个换行符后到最后一个换行符之间的内容)，并返回是否是终止指令（bool)
  virtual bool isEnd(std::string &ir, int option = 0) const
  { // option = 0 表示检查是否是终止指令，option = 1 表示检查是否是ret指令, option = 2 表示检查ret指令是否有返回值
    // 获取最后一条指令
    std::string last_instruction = get_last_instruction(ir);

    // 检查是否是ret指令
    if (option == 1)
    {
      return last_instruction.compare(0, 3, "ret") == 0;
    }
    // 检查是否是ret指令且有返回值(可以通过ret指令的最后一个字符是否是数字来判断)
    if (option == 2)
    {
      return last_instruction.compare(0, 3, "ret") == 0 && isdigit(last_instruction.back());
    }

    // 检查是否是终止指令
    return (last_instruction.compare(0, 3, "ret") == 0) ||
           (last_instruction.compare(0, 2, "br") == 0) ||
           (last_instruction.compare(0, 4, "jump") == 0);
  }

  // args 是参数列表，默认为空
  virtual std::string callIR(std::string &func_name, std::vector<ret_value_t> args = {}) const
  {
    if (isEnd(ir))
    {
      return "";
    }
    std::string ir;
    // 查询函数返回值类型
    if (symbol_table.isVoid(func_name))
    {
      ir = "\tcall @" + func_name + "(";
    }
    else
    {
      ir = "\t%" + std::to_string(global_var_index++) + " = call @" + func_name + "(";
    }
    for (int i = 0; i < args.size(); i++)
    {
      if (args[i].second == RetType::NUMBER)
      {
        ir += std::to_string(args[i].first.number);
      }
      else if (args[i].second == RetType::INDEX)
      {
        ir += "%" + std::to_string(args[i].first.number);
      }
      else if (args[i].second == RetType::IDENT)
      {
        ir += "@" + symbol_table.getUniqueIdent(args[i].first.ident);
      }
      else if (args[i].second == RetType::ARRAYPTR)
      {
        ir += "%ptr" + std::to_string(args[i].first.number);
      }
      else if (args[i].second == RetType::ARRAY)
      {
        ir += "@" + symbol_table.getUniqueIdent(args[i].first.ident);
      } else if(args[i].second == RetType::PTR)
      {
        ir += "%ptr" + std::to_string(args[i].first.number);
      }
      else
      {
        std::cerr << "callIR: unknown arg type\n";
        assert(0);
      }
      if (i != args.size() - 1)
      {
        ir += ", ";
      }
    }
    ir += ")\n";
    return ir;
  }

  std::string localInitArrayIRHelper(const std::string &base_ptr, const std::vector<int> &indexs, const std::vector<int> &init_list, size_t &init_index, int dimIndex)
  {
    std::string ir;
    if (dimIndex == indexs.size() - 1)
    {
      for (int i = 0; i < indexs[dimIndex]; ++i)
      {
        std::string ptr = "%ptr" + std::to_string(global_ptr_index++);
        ir += "\t" + ptr + " = getelemptr " + base_ptr + ", " + std::to_string(i) + "\n";
        ir += storeIR({init_list[init_index++], RetType::NUMBER}, {global_ptr_index - 1, RetType::PTR});
      }
    }
    else
    {
      for (int i = 0; i < indexs[dimIndex]; ++i)
      {
        std::string new_base_ptr = "%ptr" + std::to_string(global_ptr_index++);
        ir += "\t" + new_base_ptr + " = getelemptr " + base_ptr + ", " + std::to_string(i) + "\n";
        ir += localInitArrayIRHelper(new_base_ptr, indexs, init_list, init_index, dimIndex + 1);
      }
    }
    return ir;
  }

  std::string localInitArrayIR(std::string ident, std::vector<int> indexs, std::vector<int> init_list)
  {
    std::string base_ptr = "@" + symbol_table.getUniqueIdent(ident);
    size_t init_index = 0;
    return localInitArrayIRHelper(base_ptr, indexs, init_list, init_index, 0);
  }

  virtual std::string allocIR(std::string ident, std::string dim = "") const
  {
    if (isEnd(ir))
    {
      return "";
    }
    if (!symbol_table.isGlobal())
    {
      if (!symbol_table.isPtr(ident))
        return "\t@" + symbol_table.getUniqueIdent(ident) + " = alloc i32\n";
      else
        return "\t@" + symbol_table.getUniqueIdent(ident) + " = alloc " + dim + "\n";
    }

    return "global @" + symbol_table.getUniqueIdent(ident) + " = alloc i32, "; // global variable, 不写换行, 后面会写初始化
  }

  virtual std::string allocArrayIR(std::string ident = "", std::vector<int> size = {}, std::vector<int> init_list = {})
  {
    if (isEnd(ir))
    {
      return "";
    }
    // Helper function to generate nested initialization list
    std::function<std::string(const std::vector<int> &, size_t, size_t &)> generateNestedInitList;
    generateNestedInitList = [&](const std::vector<int> &init_list, size_t dimIndex, size_t &pos) -> std::string
    {
      if (dimIndex == size.size() - 1)
      {
        std::string result = "{";
        for (int i = 0; i < size[dimIndex]; ++i)
        {
          result += std::to_string(init_list[pos++]);
          if (i != size[dimIndex] - 1)
          {
            result += ", ";
          }
        }
        result += "}";
        return result;
      }
      else
      {
        std::string result = "{";
        for (int i = 0; i < size[dimIndex]; ++i)
        {
          result += generateNestedInitList(init_list, dimIndex + 1, pos);
          if (i != size[dimIndex] - 1)
          {
            result += ", ";
          }
        }
        result += "}";
        return result;
      }
    };

    std::string ir;
    if (!symbol_table.isGlobal())
    {
      ir = "\t@" + symbol_table.getUniqueIdent(ident) + " = alloc ";
    }
    else
    {
      ir = "global @" + symbol_table.getUniqueIdent(ident) + " = alloc ";
    }

    // 生成数组的维度部分
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

    if (symbol_table.isGlobal())
    {
      ir += ", ";
      if (init_list.size() == 0)
      {
        ir += "zeroinit\n";
        return ir;
      }
      else
      {
        size_t pos = 0;
        ir += generateNestedInitList(init_list, 0, pos) + "\n";
        return ir;
      }
    }

    ir += "\n";
    if (init_list.size() == 0)
    {
      return ir;
    }
    else
    {
      // 局部变量初始化，使用getelemptr指令和store指令：TODO
      ir += this->localInitArrayIR(ident, size, init_list);
      return ir;
    }
  }

  virtual std::string getelemptrIR(std::string ident, std::vector<ret_value_t> indexs) const
  {
    if (isEnd(ir))
    {
      return "";
    }
    if (!symbol_table.isArray(ident))
    {
      std::cerr << "getelemptrIR: " << ident << " is not an array\n";
      assert(0);
    }
    std::string ir;
    // 如果indexs为空，直接返回数组的首地址
    if (indexs.size() == 0)
    {
      // ir += "\t%ptr" + std::to_string(global_ptr_index++) + " = getelemptr @" + symbol_table.getUniqueIdent(ident) + ", 0\n";
      return ir;
    }
    // 否则，根据indexs的大小，逐层计算getelemptr
    // 处理第一维度
    // 判断indexs[0]是否是Number or Index
    if (indexs[0].second == RetType::NUMBER)
    {
      ir += "\t%ptr" + std::to_string(global_ptr_index++) + " = getelemptr @" + symbol_table.getUniqueIdent(ident) + ", " + std::to_string(indexs[0].first.number) + "\n";
    }
    else if (indexs[0].second == RetType::INDEX)
    {
      ir += "\t%ptr" + std::to_string(global_ptr_index++) + " = getelemptr @" + symbol_table.getUniqueIdent(ident) + ", %" + std::to_string(indexs[0].first.number) + "\n";
    }
    else
    {
      std::cerr << "getelemptrIR: unknown index type\n";
      assert(0);
    }

    // 处理后续维度
    for (int i = 1; i < indexs.size(); i++)
    {
      if (indexs[i].second == RetType::NUMBER)
      {
        ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getelemptr %ptr" + std::to_string(global_ptr_index - 1) + ", " + std::to_string(indexs[i].first.number) + "\n";
      }
      else if (indexs[i].second == RetType::INDEX)
      {
        ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getelemptr %ptr" + std::to_string(global_ptr_index - 1) + ", %" + std::to_string(indexs[i].first.number) + "\n";
      }
      else
      {
        std::cerr << "getelemptrIR: unknown index type\n";
        assert(0);
      }
      global_ptr_index++;
    }
    return ir;

    /* // 处理第一维度
    if(!symbol_table.isPtr(ident))
      ir += "\t%ptr" + std::to_string(global_ptr_index++) + " = getelemptr @" + symbol_table.getUniqueIdent(ident) + ", " + std::to_string(indexs[0]) + "\n";
    else{
      ir += loadIR({RetValue(ident), RetType::ARRAYPTR});
      ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getptr %ptr" + std::to_string(global_ptr_index - 1) + ", " + std::to_string(indexs[0]) + "\n";
      global_ptr_index++;
    }


    // 处理后续维度
    for(int i = 1; i < indexs.size(); i++) {
      ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getelemptr %ptr" + std::to_string(global_ptr_index - 1) + ", " + std::to_string(indexs[i]) + "\n";
      global_ptr_index++;
    }
    return ir; */
  }

  virtual std::string getptrIR(std::string ident, std::vector<ret_value_t> indexs) const
  {
    if (isEnd(ir))
    {
      // std::cerr << "getptrIR: ir is end\n";
      return "";
    }
    if (!symbol_table.isPtr(ident))
    {
      std::cerr << "getptrIR: " << ident << " is not a pointer\n";
      assert(0);
    }
    std::string ir;
    // load pointer

    ir += loadIR({RetValue(ident), RetType::ARRAYPTR});
    if(indexs.size() == 0)
    {
      std::cout << "getptrIR: indexs is empty\n";
      return ir;
    }
    // 处理第一维度
    if (indexs[0].second == RetType::NUMBER)
    {
      ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getptr %ptr" + std::to_string(global_ptr_index - 1) + ", " + std::to_string(indexs[0].first.number) + "\n";
    }
    else if (indexs[0].second == RetType::INDEX)
    {
      ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getptr %ptr" + std::to_string(global_ptr_index - 1) + ", %" + std::to_string(indexs[0].first.number) + "\n";
    }
    else
    {
      std::cerr << "getptrIR: unknown index type\n";
      assert(0);
    }
    global_ptr_index++;
    // 处理后续维度，使用getelemptr
    for (int i = 1; i < indexs.size(); i++)
    {
      if (indexs[i].second == RetType::NUMBER)
      {
        ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getelemptr %ptr" + std::to_string(global_ptr_index - 1) + ", " + std::to_string(indexs[i].first.number) + "\n";
      }
      else if (indexs[i].second == RetType::INDEX)
      {
        ir += "\t%ptr" + std::to_string(global_ptr_index) + " = getelemptr %ptr" + std::to_string(global_ptr_index - 1) + ", %" + std::to_string(indexs[i].first.number) + "\n";
      }
      else
      {
        std::cerr << "getptrIR: unknown index type\n";
        assert(0);
      }
      global_ptr_index++;
    }
    return ir;
    
  }
};

typedef std::vector<std::pair<ListType, std::unique_ptr<BaseAST>>> List;
