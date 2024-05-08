#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include "/root/compiler/sysy-make-template/ast/ast.hh"
using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  // 输出解析得到的 AST, 其实就是个字符串
  cout << endl;
  ast->Dump();
  cout << endl << endl;

  // .dot
  string dot = "digraph G {\n";
  dot += "node [shape = record,height=.1]\n";
  dot += "node0[label = \"<f0> CompUnit\"];\n";

  dot += "\"node0\":f0 ->\"" + ast->getUniqueID() + "\";\n";
  ast->toDot(dot);
  dot += "}\n";
  cout << dot << endl;

  // 创建或覆盖 Tree.dot 文件
    string dotFilePath = "./plot/Tree.dot";
    ofstream dotFile(dotFilePath, ios::out | ios::trunc);
    if (dotFile.is_open()) {
        dotFile << dot;
        dotFile.close();
    } else {
        cerr << "无法打开文件 " << dotFilePath << endl;
        return -1;
    }

    // 调用系统命令生成 PNG 图像文件
    string cmd = "dot -Tpng " + dotFilePath + " -o ./plot/Tree.png";
    int systemRet = system(cmd.c_str());
    if (systemRet != 0) {
        cerr << "命令执行失败: " << cmd << endl;
        return -1;
    }

    cout << "AST 已经被成功输出到 /plot/Tree.png" << endl;

  return 0;
}
