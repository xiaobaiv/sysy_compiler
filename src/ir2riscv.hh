#include <iostream>
#include <string>
#include <cassert>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "reg_manager.hh"
#include "koopa.h"

bool debug = false;
// regmanager
RegManager regManager;

std::unordered_map<koopa_raw_value_t, std::string> value2reg;

// 函数声明
void visit(const koopa_raw_program_t &program, std::stringstream &riscv);
void visit(const koopa_raw_slice_t &slice, std::stringstream &riscv);
void visit(const koopa_raw_function_t &func, std::stringstream &riscv);
void visit(const koopa_raw_basic_block_t &bb, std::stringstream &riscv);
std::string visit(const koopa_raw_value_t &value, std::stringstream &riscv);
void visit(const koopa_raw_return_t &ret, std::stringstream &riscv);
std::string visit(const koopa_raw_integer_t &integer, std::stringstream &riscv);
std::string visit(const koopa_raw_binary_t &binary, std::stringstream &riscv);

// 将Koopa IR转换为RISC-V汇编代码并返回字符串
std::string ir2riscv(const std::string &ir) {
    if (debug) std::cout << "Function: ir2riscv" << std::endl;
    std::stringstream riscv;

    // 解析字符串 str, 得到 Koopa IR 程序
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(ir.data(), &program);
    assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错

    // 创建一个 raw program builder, 用来构建 raw program
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    // 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);

    // 处理 raw program
    visit(raw, riscv);

    // 处理完成, 释放 raw program builder 占用的内存
    // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
    // 所以不要在 raw program 处理完毕之前释放 builder
    koopa_delete_raw_program_builder(builder);

    return riscv.str();
}

// 访问程序
void visit(const koopa_raw_program_t &program, std::stringstream &riscv) {
    if (debug) std::cout << "Function: visit (program)" << std::endl;
    // visit(program.values, riscv); // 访问所有全局变量
    visit(program.funcs, riscv);  // 访问所有函数
}

// 访问 raw slice
void visit(const koopa_raw_slice_t &slice, std::stringstream &riscv) {
    if (debug) std::cout << "Function: visit (slice)" << std::endl;
    if (debug) std::cout << "slice.len: " << slice.len << std::endl;



    for (size_t i = 0; i < slice.len; i++) {
        auto ptr = slice.buffer[i];
        // 根据 slice 的 kind 决定将 ptr 视作何种元素
        switch (slice.kind) {
            case KOOPA_RSIK_FUNCTION:
                // 访问函数
                visit(reinterpret_cast<koopa_raw_function_t>(ptr), riscv);
                break;
            case KOOPA_RSIK_BASIC_BLOCK:
                // 访问基本块
                visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr), riscv);
                break;
            case KOOPA_RSIK_VALUE:
                // 访问指令
                visit(reinterpret_cast<koopa_raw_value_t>(ptr), riscv);
                break;
            default:
                // 我们暂时不会遇到其他内容, 于是不对其做任何处理
                assert(false);
        }
    }
}

// 访问函数
void visit(const koopa_raw_function_t &func, std::stringstream &riscv) {
    if (debug) std::cout << "Function: visit (function)" << std::endl;
    // 获取函数名并输出函数名等信息
    std::string func_name = func->name + 1; // 去掉名字前的@
    riscv << " .text\n";
    riscv << " .globl " << func_name << "\n";
    riscv << func_name << ":\n";

    // 访问所有基本块
    visit(func->bbs, riscv);
}

// 访问基本块
void visit(const koopa_raw_basic_block_t &bb, std::stringstream &riscv) {
    if (debug) std::cout << "Function: visit (basic block)" << std::endl;
    // 访问所有指令
    visit(bb->insts, riscv);
}

// 访问指令
std::string visit(const koopa_raw_value_t &value, std::stringstream &riscv) {
    if (debug) std::cout << "Function: visit (value)" << std::endl;

    if(value2reg.find(value) != value2reg.end()) {
        return value2reg[value];
    }
    std::string reg;
    const auto &kind = value->kind;
    switch (kind.tag) {
        case KOOPA_RVT_RETURN:
            // 访问 return 指令
            visit(kind.data.ret, riscv);
            reg = "";
            break;
        case KOOPA_RVT_INTEGER:
            // 访问 integer 指令
            reg = visit(kind.data.integer, riscv);
            value2reg[value] = reg;
            break;
        case KOOPA_RVT_BINARY:
            // 访问 binary 指令
            reg = visit(kind.data.binary, riscv);
            value2reg[value] = reg;
            break;
        default:
            // 其他类型暂时遇不到
            assert(false);
    }
    return reg;
}

// 访问返回指令
void visit(const koopa_raw_return_t &ret, std::stringstream &riscv) {
    if (debug) std::cout << "Function: visit (return)" << std::endl;
    if (ret.value != NULL) {
        // 有返回值
        std::string result = visit(ret.value, riscv);
        if(result != "a0") {
            riscv << " mv a0, " << result << "\n";
            regManager.freeReg(result);
        }
    }
    riscv << " ret\n";
}

// 访问整数指令
std::string visit(const koopa_raw_integer_t &integer, std::stringstream &riscv) {
    if (debug) std::cout << "Function: visit (integer)" << std::endl;
    int32_t int_val = integer.value;
    if(int_val == 0) {
        return regManager.get0Reg();
    }
    std::string reg = regManager.getFreeReg();
    if(debug) std::cout << "reg: " << reg << std::endl;
    riscv << " li " << reg << ", " << int_val << "\n";
    return reg;
}

// 访问二元操作指令
std::string visit(const koopa_raw_binary_t &binary, std::stringstream &riscv) {
    if (debug) std::cout << "Function: visit (binary)" << std::endl;

     std::string lhs = visit(binary.lhs, riscv);
     std::string rhs = visit(binary.rhs, riscv);
     switch (binary.op) {
        case KOOPA_RBO_EQ: {
            regManager.freeReg(rhs);
            regManager.freeReg(lhs);
            std::string reg = regManager.getFreeReg();
            riscv << " xor " << reg << ", " << lhs << ", " << rhs << "\n";
            regManager.freeReg(reg);
            std::string reg2 = regManager.getFreeReg();
            riscv << " seqz " << reg2 << ", " << reg << "\n";
            return reg2;
        }
        case KOOPA_RBO_SUB: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " sub " << reg << ", " << lhs << ", " << rhs << "\n";
          return reg;
        }
        case KOOPA_RBO_ADD: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " add " << reg << ", " << lhs << ", " << rhs << "\n";
          return reg;
        }
        case KOOPA_RBO_MUL: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " mul " << reg << ", " << lhs << ", " << rhs << "\n";
          return reg;
        }
        case KOOPA_RBO_DIV: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " div " << reg << ", " << lhs << ", " << rhs << "\n";
          return reg;
        }
        case KOOPA_RBO_MOD: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " rem " << reg << ", " << lhs << ", " << rhs << "\n";
          return reg;
        }
        case KOOPA_RBO_LE: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " sgt " << reg << ", " << lhs << ", " << rhs << "\n";
          regManager.freeReg(reg);
          std::string reg2 = regManager.getFreeReg();
          riscv << " seqz " << reg2 << ", " << reg << "\n";
          return reg2;
        }
        case KOOPA_RBO_LT: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " slt " << reg << ", " << lhs << ", " << rhs << "\n";
          return reg;
        }
        case KOOPA_RBO_GE: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " slt " << reg << ", " << lhs << ", " << rhs << "\n";
          regManager.freeReg(reg);
          std::string reg2 = regManager.getFreeReg();
          riscv << " seqz " << reg2 << ", " << reg << "\n";
          return reg2;
        }
        case KOOPA_RBO_GT: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " sgt " << reg << ", " << lhs << ", " << rhs << "\n";
          return reg;
        }
        case KOOPA_RBO_NOT_EQ: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " xor " << reg << ", " << lhs << ", " << rhs << "\n";
          regManager.freeReg(reg);
          std::string reg2 = regManager.getFreeReg();
          riscv << " snez " << reg2 << ", " << reg << "\n";
          return reg2;
        }
        case KOOPA_RBO_AND: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " and " << reg << ", " << lhs << ", " << rhs << "\n";
          return reg;
        }
        case KOOPA_RBO_OR: {
          regManager.freeReg(rhs);
          regManager.freeReg(lhs);
          std::string reg = regManager.getFreeReg();
          riscv << " or " << reg << ", " << lhs << ", " << rhs << "\n";
          return reg;
        }
        default:
            std::cerr << "unhandled binary operator: " << binary.op << std::endl;
            assert(false);
    } 

}


