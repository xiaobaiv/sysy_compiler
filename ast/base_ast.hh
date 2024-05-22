#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <assert.h>

enum class RetType { NUMBER, INDEX, VOID };
typedef std::pair<int, RetType> ret_value_t;
typedef uint64_t var_index_t;
enum class ListType { CONSTDEF,VARDEF ,DECL, STMT, BLOCKITEM, FUNCFPARAM, EXP, CONSTEXP, INITVAL, CONSTINITVAL };
// 所有 AST 的基类
class BaseAST {
  public:
  static var_index_t global_var_index;
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;

  virtual void toDot(std::string& dot) const = 0;

  virtual ret_value_t toIR(std::string &ir) {
    ir += "toIR not implemented\n";
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
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " " + std::to_string(ret1.first) + ", " + std::to_string(ret2.first) + "\n";
      } else if(ret2.second == RetType::INDEX) {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " " + std::to_string(ret1.first) + ", %" + std::to_string(ret2.first) + "\n";
      } else {
        std::cerr << "getIR: unknown ret2 type\n";
      }
    } else if(ret1.second == RetType::INDEX) {
      if(ret2.second == RetType::NUMBER) {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " %" + std::to_string(ret1.first) + ", " + std::to_string(ret2.first) + "\n";
      } else if(ret2.second == RetType::INDEX) {
        ir = "\t%" + std::to_string(global_var_index++) + " = " + op + " %" + std::to_string(ret1.first) + ", %" + std::to_string(ret2.first) + "\n";
      } else {
        std::cerr << "getIR: unknown ret2 type\n";
      }
    } else {
      std::cerr << "getIR: unknown ret1 type\n";
    }
    return ir;
  }

};

typedef std::vector<std::pair<ListType, std::unique_ptr<BaseAST>>> List;



