#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <assert.h>
#include <unordered_map>
#include <algorithm>
#include "sysbol_table.hh"


enum class RetType { NUMBER, INDEX, VOID, IDENT };
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
enum class ListType { CONSTDEF,VARDEF ,DECL, STMT, BLOCKITEM, FUNCFPARAM, EXP, CONSTEXP, INITVAL, CONSTINITVAL };
// 所有 AST 的基类
class BaseAST {
  public:
  static var_index_t global_var_index;
  static var_index_t global_label_index;
  // 符号表
  // static std::unordered_map<std::string, int> symbol_table;
  static SymbolTable symbol_table;
  static std::string ir;
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;

  virtual void toDot(std::string& dot) const = 0;
  virtual int calc() {
    std::cerr << "calc not implemented\n";
    assert(0);
  }
  virtual ret_value_t toIR(std::string &ir) {
    std::cerr << "toIR not implemented\n";
    assert(0);
  }
  // 为了区分, 我们为每个 AST 节点(label)生成一个唯一的 ID
  virtual std::string getUniqueID() const {
    return "Node_" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
  }
  virtual std::string getIR(std::string op, ret_value_t ret1, ret_value_t ret2) const {
    if(isEnd(ir)) {
      return "";
    }
    std::string ir;
    if(ret1.second == RetType::NUMBER) {
      if(ret2.second == RetType::NUMBER) {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " " + std::to_string(ret1.first.number) + ", " + std::to_string(ret2.first.number) + "\n";
      } else if(ret2.second == RetType::INDEX) {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " " + std::to_string(ret1.first.number) + ", %" + std::to_string(ret2.first.number) + "\n";
      } else {
        std::cerr << "getIR: unknown ret2 type\n";
      }
    } else if(ret1.second == RetType::INDEX) {
      if(ret2.second == RetType::NUMBER) {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " %" + std::to_string(ret1.first.number) + ", " + std::to_string(ret2.first.number) + "\n";
      } else if(ret2.second == RetType::INDEX) {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " %" + std::to_string(ret1.first.number) + ", %" + std::to_string(ret2.first.number) + "\n";
      } else {
        std::cerr << "getIR: unknown ret2 type\n";
      }
    } else {
      std::cerr << "getIR: unknown ret1 type\n";
    }
    return ir;
  }
  virtual std::string storeIR(ret_value_t ret1, ret_value_t ret2) const {
    if(isEnd(ir)) {
      return "";
    }
    std::string ir;
    // ret2.second must be INDENT
    if(ret1.second == RetType::NUMBER) {
      ir = "\tstore " + std::to_string(ret1.first.number) + ", @" + symbol_table.getUniqueIdent(ret2.first.ident) + "\n";
    } else if(ret1.second == RetType::INDEX) {
      ir = "\tstore %" + std::to_string(ret1.first.number) + ", @" + symbol_table.getUniqueIdent(ret2.first.ident) + "\n";
    } else {
      std::cerr << "storeIR: unknown ret1 type\n";
      assert(0);
    }
    return ir;
  }
  virtual std::string loadIR(ret_value_t ret) const {
    if(isEnd(ir)) {
      return "";
    }
    std::string ir;
    if(ret.second == RetType::IDENT) {
      ir = "\t%" + std::to_string(global_var_index++) + " = load @" + symbol_table.getUniqueIdent(ret.first.ident) + "\n";
    } else {
      std::cerr << "loadIR: unknown ret type\n";
      assert(0);
    }
    return ir;
  }
  virtual std::string allocIR(std::string ident) const {
    if(isEnd(ir)) {
      return "";
    }
    std::string ir = "\t@" + symbol_table.getUniqueIdent(ident) + " = alloc i32\n";
    return ir;
  }
  virtual std::string brIR(ret_value_t ret, std::string label1, std::string label2) const {
    if(isEnd(ir)) {
      return "";
    }
    std::string ir;
    if(ret.second == RetType::NUMBER) {
      ir = "\tbr " + std::to_string(ret.first.number) + ", %" + label1 + ", %" + label2 + "\n";
    } else if (ret.second == RetType::INDEX) {
      ir = "\tbr %" + std::to_string(ret.first.number) + ", %" + label1 + ", %" + label2 + "\n";
    } else {
      std::cerr << "brIR: unknown ret type\n";
      assert(0);
    }
    return ir;
  }
  virtual std::string labelIR(std::string label) const {
    return "%" + label + ":\n";
  }
  virtual std::string jumpIR(std::string label) const {
    if(isEnd(ir)) {
      return "";
    }
    return "\tjump %" + label + "\n";
  }

  // 为了防止在生成 IR 时在终止指令（ret、br、jump）之后还有其他指令，我们需要从ir中提取出最后一条指令，当前ir的最后一个字符一定是换行符(即倒数第二个换行符后到最后一个换行符之间的内容)，并返回是否是终止指令（bool)
  virtual bool isEnd(std::string &ir) const {
        // 从最后一个换行符开始向前找倒数第二个换行符
        size_t last_newline = ir.find_last_of('\n');
        if (last_newline == std::string::npos || last_newline == 0) {
            // 如果没有找到换行符或者只有一个换行符，则说明没有有效的指令
            return false;
        }

        size_t second_last_newline = ir.find_last_of('\n', last_newline - 1);
        if (second_last_newline == std::string::npos) {
            // 如果找不到倒数第二个换行符，说明只有一行指令
            second_last_newline = 0;
        } else {
            // 否则将第二个换行符位置移到下一个字符，开始提取最后一条指令
            second_last_newline += 1;
        }

        // 提取最后一条指令
        std::string last_instruction = ir.substr(second_last_newline, last_newline - second_last_newline);

        // 检查是否是终止指令
        last_instruction.erase(std::remove_if(last_instruction.begin(), last_instruction.end(), ::isspace), last_instruction.end());
        return (last_instruction.compare(0, 3, "ret") == 0) ||
               (last_instruction.compare(0, 2, "br") == 0) ||
               (last_instruction.compare(0, 4, "jump") == 0);
    }



};

typedef std::vector<std::pair<ListType, std::unique_ptr<BaseAST>>> List;



