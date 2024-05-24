#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <assert.h>
#include <unordered_map>
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
  // 符号表
  // static std::unordered_map<std::string, int> symbol_table;
  static SymbolTable symbol_table;
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
  // 为了区分, 我们为每个 AST 节点生成一个唯一的 ID
  virtual std::string getUniqueID() const {
        return "Node_" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
    }
  virtual std::string getIR(std::string op, ret_value_t ret1, ret_value_t ret2) const {
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
    std::string ir = "\t@" + symbol_table.getUniqueIdent(ident) + " = alloc i32\n";
    return ir;
  }
};

typedef std::vector<std::pair<ListType, std::unique_ptr<BaseAST>>> List;



