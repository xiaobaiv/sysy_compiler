#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include "koopa.h"
#include "/root/compiler/sysy-make-template/ast/ast.hh"
#include "ir2riscv.hh"
using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int genDot(unique_ptr<BaseAST> &ast) {
  // .dot
  string dot = "digraph G {\n";
  dot += "node [shape = record,height=.1]\n";
  dot += "node0[label = \"<f0> CompUnit\"];\n";

  dot += "\"node0\":f0 ->\"" + ast->getUniqueID() + "\";\n";
  ast->toDot(dot);
  dot += "}\n";
  //cout << dot << endl;

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

int main(int argc, char *argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " [-dot] mode input_file -o output_file" << endl;
        return -1;
    }

    bool generateDot = false;
    string mode, input, output;
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-dot") {
            generateDot = true;
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                output = argv[++i];
            }
        } else if (mode.empty()) {
            mode = arg;
        } else if (input.empty()) {
            input = arg;
        }
    }

    if (input.empty() || output.empty()) {
        cerr << "Invalid command line. Make sure to include mode, input file, and output file." << endl;
        return -1;
    }

    yyin = fopen(input.c_str(), "r");
    if (!yyin) {
        cerr << "Cannot open input file: " << input << endl;
        return -1;
    }

    unique_ptr<BaseAST> ast;
    if (yyparse(ast) != 0) {
        cerr << "Failed to parse input file." << endl;
        return -1;
    }

    if (generateDot) {
        genDot(ast); // Only call if -dot is specified
    }


    ast->toIR(ast->ir);
    string ir = ast->ir;
    if (mode == "-koopa") {
        ofstream irFile(output, ios::out | ios::trunc);
        if (irFile.is_open()) {
            irFile << ir;
            irFile.close();
        } else {
            cerr << "Cannot open output file: " << output << endl;
            return -1;
        }
    } else if (mode == "-riscv" || mode == "-perf") {
        string riscv = ir2riscv(ir);
        ofstream riscvFile(output, ios::out | ios::trunc);
        if (riscvFile.is_open()) {
            riscvFile << riscv;
            riscvFile.close();
        } else {
            cerr << "Cannot open output file: " << output << endl;
            return -1;
        }
    }

    return 0;
}