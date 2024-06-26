## 问题与解决方案记录(开发日志)

### 2024/5/21

#### Q0: 多个文件中的子类如何共享一个父类的变量（类似全局变量的效果）？
**A0:** 通过将父类的变量设置为静态变量，然后通过父类的静态方法来访问这个变量。这样就可以实现多个子类共享一个父类的变量的效果。需要注意的是，需要在 `base_ast.cpp` 中定义（只1次）这个静态变量。还需在原有的makefile中将ast文件夹中的源文件加入到编译中，否则会出现未定义的错误。

### 2024/5/22

#### Q1: Koopa IR 的内存形式中，block包含多个value，value可以看做是一系列指令的集合，如何遍历这些指令更好？
**A1:** 
- **v0:** 遍历slice，同时在binary中重复visit op 的数据对象。这就会导致重复生成目标代码。
- **v1:** 分析block可知，其中的指令是线性分布的，也就是可以看做棵树，树根是最后一个指令。优点是无需引入额外的数据结构，缺点是会造成寄存器的挤压（树的dfs实际上用到了栈），此外，有个悬而未决的问题，就是是否对块的跳转是透明的（应该是的）。
- **v2:** 引入unordered_map, 记录指令结果，避免重复visit，生成重复目标代码。
- **补充（2024/5/23）:** 通过阅读文档优化部分Lv9+.3部分，发现v1方法可以实现无关代码的删除（aka dead code elimination），这样可以减少目标代码的长度，提高程序的运行效率。

#### Q2: 寄存器分配的问题，如何解决？
**A2:** 引入寄存器管理器RegManager封装，使得寄存器的分配更加方便，同时也可以避免寄存器的挤压。同时，也便于后续分配算法的实现（如果有时间的话）。

#### Q3: 找寻寄存器分配的规律？
**A3:** 通过观察，发现寄存器分配的规律是，如果一个寄存器存储的结果参与运算，则这个寄存器可以被释放。通常，释放后的寄存器会被下一个指令使用。这样就可以实现寄存器的复用。

#### Q4: 如何判定什么时候使用addi, 什么时候使用add？
**A4:** 为了简便，所有的立即数都需要先存储到寄存器中，然后再进行运算。当然，这样会造成寄存器的浪费。但是，这样可以简化指令的选择。如果有时间的话，可以考虑优化这一部分。

#### Q5: koopa IR 不支持逻辑与和逻辑或，如何解决？
**A5:** koopa IR 不支持逻辑或，需要按位或，然后与0比较，得到结果；koopa IR 不支持两个数直接与，需要判断两个数是否都不为0，然后按位与。

### 2024/5/23

#### T6: 为了处理变量（Lv4）IR 的生成，为ret_value_t 增加 IDENT 类别，特别地，需要将 LVal 与 load 指令解耦合，在 caller 处进行判断，若 LVal 为等号左值，无需load，直接 store, 否则load。

#### Q7: 语义检查很繁琐，对函数的实现有着逻辑缜密的要求。是否可以将其与开发解耦合？或者是否可以使用理论模型（maybe 图）使得语义检查（包括异常管理）思路清晰？

### 2024/5/24

#### Q8: 如何管理多个作用域嵌套的符号表？

#### A8: 使用栈结构。相应地，为了适应 Koopa IR symbol 的不可复用性，符号表类提供一个接口getUniqueIdent()，实现原理是：每个作用域出现的符号表都有一个单独的标号（由SymbolTable在push()时维护），UniqueIdent() = ident + "_" + table.index。需要注意的是：符号表中ident（键值）不受后缀影响。全局作用域index=0
```c
# 效果如下
fun @main(): i32 {
%entry:
	@x_1 = alloc i32 #  int main() {
	store 10, @x_1	 #  	int x = 10;
	%0 = load @x_1   #  	x = x + 1;
	%1 = add %0, 1   #  	return x;
	store %1, @x_1   #  }
	%2 = load @x_1   #
	ret %2
}

```

#### Q9: 遇到了一个新问题，我是在Block里创建新的符号表，还是在调用Block时创建？

#### A9: 前者虽然工整，但是如果是函数的Block，那么函数的参数要像C一样，归属于block的作用域，有矛盾，需要引入新的数据结构。

#### Q10: 接着Q9，考虑函数调用，符号表是否是短暂地弹出，是否需要回收（保存现场），然后再次压入？

#### A10： 不需要，这是一个误区，幸亏吃了口泡面，不然就写错了。 我们只是负责生成IR代码，并不是执行代码，所以扫描是线性的。
```c
int a = 10;
int test(int x, int y) {
  return x + y + a;

}

int main() {
  int a = 1;
  int b = test(2, 3);
  return b;
}

```
对应下方IR:
```c

global @a_00 = alloc i32, 10
fun @test(@x_test: i32, @y_test: i32): i32 {
%entry:
  %x_10 = alloc i32
  store @x_test, %x_10
  %y_10 = alloc i32
  store @y_test, %y_10
  %0 = load %x_10
  %1 = load %y_10
  %2 = add %0, %1
  %3 = load @a_00
  %4 = add %2, %3
  ret %4
}
fun @main(): i32 {
%entry:
  @a_21 = alloc i32
  store 1, @a_21
  @b_21 = alloc i32
  %5 = call @test(2, 3)
  store %5, @b_21
  %6 = load @b_21
  ret %6
}
```

#### Q11: 刚遇到了一个贼傻逼的错误，无论const decl在哪层嵌套，都会被放到全局的符号表里，原因竟是？

#### A11: 原因在于我一开始把常量声明的符号表插入操作放在了constdef的构造函数里（因为常量声明实际上不生成任何IR，我就省略了子树的toIR（）函数，结果弄巧成拙），也就是说在语法分析的时候就插到符号表里了，而这时只有全局符号表。

### 2024/5/25

#### Q12: 如何解决一个基本块（IR的block）只出现一次终结指令（ret、jump、br）的问题？

#### A12: 引入isEnd(ir)函数，从ir中读取最后一个指令判断是否是终结指令，并在storeIR等函数入口调用。特别地，为了充分利用封装（比如storeIR）的优势，避免在全部代码中更改每一次可能出现的非labelIR()指令，将ir设计成baseIR的static变量，并在main.cpp里将其作为toIR()的参数（很丑陋，有时间再改吧，1. 将toIR处理对象改为static ir，而非main.cpp传入的参数。 2. 增加一个返回baseIR static ir 引用的接口，避免拷贝、提高性能）。

#### T13: 短路求值时，编译器相当于创建临时变量result作为表达式结果，比较naive的办法是直接插入符号表result变量，但有可能和符号表中已有的result有冲突，所以不可取。比较好的做法是巧用作用域的概念，在处理 || and && 表达式时，创建临时作用域，避免 Koopa IR 的 symbol 命名冲突。

#### Q14: 短路求值时，如果遇到 1 || 2 || 3 这样的表达式，如何保证先看 从左到右进行判0呢？