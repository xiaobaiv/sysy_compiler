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

  virtual void toDot(std::string& dot) const = 0;

  // 为了区分, 我们为每个 AST 节点生成一个唯一的 ID
  virtual std::string getUniqueID() const {
        return "Node_" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
    }
};

typedef std::vector<std::pair<ListType, std::unique_ptr<BaseAST>>> List;