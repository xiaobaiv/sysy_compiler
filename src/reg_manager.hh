#include <vector>
#include <string>
#include <map>
#include <stack>
#include "koopa.h"

class RegManager {
    enum class Register {
        t0, t1, t2, t3, t4, t5, t6, a0, a1, a2, a3, a4, a5, a6, a7, x0
    };

private:
    std::stack<Register> free_regs; // 用于存储空闲的寄存器
    std::map<Register, bool> reg_used; // 用于存储寄存器是否被使用
    std::map<Register, std::string> reg_name; // 用于存储寄存器的名字
    std::map<std::string, Register> name_reg; // 用于存储名字对应的寄存器

    Register _getFreeReg();
    void _freeReg(Register reg);

public:
    RegManager() {
        // 使用初始化列表和标准库的insert方法初始化堆栈
        std::initializer_list<Register> regs = {
            Register::a7, Register::a6, Register::a5, Register::a4,
            Register::a3, Register::a2, Register::a1, Register::a0,
            Register::t6, Register::t5, Register::t4, Register::t3,
            Register::t2, Register::t1, Register::t0
        };
        for (auto reg : regs) {
            free_regs.push(reg);
            reg_used[reg] = false;
        }
        reg_used[Register::x0] = true;

        std::vector<std::pair<Register, std::string>> reg_names = {
            {Register::t0, "t0"}, {Register::t1, "t1"}, {Register::t2, "t2"},
            {Register::t3, "t3"}, {Register::t4, "t4"}, {Register::t5, "t5"},
            {Register::t6, "t6"}, {Register::a0, "a0"}, {Register::a1, "a1"},
            {Register::a2, "a2"}, {Register::a3, "a3"}, {Register::a4, "a4"},
            {Register::a5, "a5"}, {Register::a6, "a6"}, {Register::a7, "a7"},
            {Register::x0, "x0"}
        };

        for (const auto& pair : reg_names) {
            reg_name[pair.first] = pair.second;
            name_reg[pair.second] = pair.first;
        }
    }

    std::string get0Reg() {
        return "x0";
    }

    std::string getFreeReg() {
        Register reg = _getFreeReg();
        return reg_name[reg];
    }

    void freeReg(std::string reg) {
        if(reg == "x0") {
            return;
        }
        Register r = name_reg[reg];
        _freeReg(r);
    }
};

RegManager::Register RegManager::_getFreeReg() {
    if (free_regs.empty()) {
        throw std::runtime_error("No free registers available");
    }
    Register reg = free_regs.top();
    free_regs.pop();
    reg_used[reg] = true;
    return reg;
}

void RegManager::_freeReg(Register reg) {
    if (!reg_used[reg]) {
        throw std::runtime_error("Trying to free a register that is not in use " + reg_name[reg]);
    }
    reg_used[reg] = false;
    free_regs.push(reg);
}
