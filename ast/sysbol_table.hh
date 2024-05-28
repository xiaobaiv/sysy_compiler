#pragma once
#include <vector>
#include <unordered_map>
#include <map>
// 
/* 
* this class is a symbol table for the compiler, the table is a vector of items, 
* each item has a ident(string), type(enum class), and value(depend on the type, implemented by union)
*  
* functions:
* 1. insert: insert a new symbol to the table, return true if success, otherwise return false
* 2. find: find a symbol in the table, return the item if found, otherwise return nullptr
* 
* data structure:
* item: enum class Type {CONST, VAR} , value(int) 目前只有int类型，引入数组后需要修改
* single_table: map<string, item>
* sysbol_table: // 为了实现作用域嵌套，理论上使用栈的数据结构更加方便。但是这里使用vector来实现，因为vector可以方便访问{上一层}的符号表，栈底是全局符号表
* let's start!
 */class Item {
public:
    enum class Type {CONST, VAR, FUNC, CARRAY, VARRAY, PTR};
    Type type;
    int value;
    Item(Type type, int value): type(type), value(value) {} // 如果是函数的话，value表示FuncType，如果FuncType是0表示void，否则表示int类型, 如果是数组的话，value表示数组的维度长度（方括号的个数）
    Item() {}
    bool isConst() { return type == Type::CONST; }
    bool isVar() { return type == Type::VAR; }
    bool isFunc() { return type == Type::FUNC; }
    bool isPtr() { return type == Type::PTR; }
    bool isArray() { return type == Type::CARRAY || type == Type::VARRAY;}
    int getValue() { return value; }
};

class SingleTable {
public:
    std::map<std::string, Item> table;
    int index;
    SingleTable(int idx) : index(idx) {} // 初始化索引值
    bool insert(std::string ident, Item item) {
        if (table.find(ident) != table.end()) return false;
        table[ident] = item;
        return true;
    }
    Item* find(std::string ident) {
        if (table.find(ident) == table.end()) return nullptr;
        return &table[ident];
    }
};

class SymbolTable {
public:
    std::vector<SingleTable> tables;
    int only_increase_index;
    SymbolTable() {
        only_increase_index = 0;
        push();
        // 在初始化时候，将sysy库函数加入符号表，decl @getint(): i32\ndecl @getch(): i32\ndecl @getarray(*i32): i32\ndecl @putint(i32)\ndecl @putch(i32)\ndecl @putarray(i32, *i32)\ndecl @starttime()\ndecl @stoptime()
        insert("getint", Item(Item::Type::FUNC, 1));
        insert("getch", Item(Item::Type::FUNC, 1));
        insert("getarray", Item(Item::Type::FUNC, 1));
        insert("putint", Item(Item::Type::FUNC, 0));
        insert("putch", Item(Item::Type::FUNC, 0));
        insert("putarray", Item(Item::Type::FUNC, 0));
        insert("starttime", Item(Item::Type::FUNC, 0));
        insert("stoptime", Item(Item::Type::FUNC, 0));
    }
    bool insert(std::string ident, Item item) {
        return tables.back().insert(ident, item);
    }
    Item* find(std::string ident) {
        for (int i = tables.size() - 1; i >= 0; i--) {
            Item* item = tables[i].find(ident);
            if (item != nullptr) return item;
        }
        return nullptr;
    }
    bool isConst(std::string ident) {
        Item* item = find(ident);
        if (item == nullptr) return false;
        return item->isConst();
    }
    bool isVar(std::string ident) {
        Item* item = find(ident);
        if (item == nullptr) return false;
        return item->isVar();
    }
    bool isFunc(std::string ident) {
        Item* item = find(ident);
        if (item == nullptr) return false;
        return item->isFunc();
    }
    int getValue(std::string ident) {
        Item* item = find(ident);
        if (item == nullptr) return 0;
        return item->getValue();
    }
    bool isVoid(std::string ident) {
        Item* item = find(ident);
        if (item == nullptr) return false;
        return item->getValue() == 0;
    }
    bool isPtr(std::string ident) {
        Item* item = find(ident);
        if (item == nullptr) return false;
        return item->isPtr();
    }
    bool isArray(std::string ident) {
        Item* item = find(ident);
        if (item == nullptr) return false;
        return item->isArray();
    }
    void push() {
        tables.emplace_back(only_increase_index++);
    }
    void pop() {
        tables.pop_back();
    }
    std::string getUniqueIdent(std::string ident) {
        // print();
        int x = 999;
        for (int i = tables.size() - 1; i >= 0; i--) {
            if (tables[i].find(ident) != nullptr) {
                x = tables[i].index;
                break;
            }
        }
        if(x == 999)
        {
            print();
        }
        return ident + "_" + std::to_string(x);
    }
    void print() {
        std::cout << "---------------------------------------------" << std::endl;
        for (int i = 0; i < tables.size(); i++) {
            std::cout << "table " << i << ", ";
            std::cout << "index " << tables[i].index << std::endl;
            for (auto it = tables[i].table.begin(); it != tables[i].table.end(); it++) {
                std::cout << it->first << " " << it->second.value << std::endl;
            }
        }
    }

    bool isGlobal() {
        return tables.back().index == 0;
    }
};