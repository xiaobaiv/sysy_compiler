# SysY Compiler 
**latest update: 2024/5/23**

本实验参考北大编译原理实验指导文档：https://pku-minic.github.io/online-doc/#/ ，目前完成了语法分析和AST的构建，使用graphviz工具进行AST的绘制，正在完成IR和目标代码的生成。

## 1. Run

```bash
# 获取项目文件
git clone https://github.com/xiaobaiv/sysy_compiler.git
# 拉取docker image
docker pull guozhengwu/compiler-dev
# 运行容器，挂载项目文件
docker run -it --rm -v <项目文件> guozhengwu/compiler-dev bash
# 在bash中执行以下命令，得到结果
make
build/compiler -riscv /root/compiler/sysy-make-template/debug/hello.c -o /root/compiler/sysy-make-template/debug/hello.riscv

```

## 2. 编译器使用帮助

这个程序用于编译SysY语言文件，并可以输出Koopa IR或RISC-V汇编代码，同时支持生成抽象语法树（AST）的图像文件。

### 2.1 使用方法

```bash
./compiler [-dot] mode input_file -o output_file
```


#### 2.1.1 参数说明

- `[-dot]` (可选): 如果提供此选项，程序将生成一个表示程序AST的图形文件（PNG格式），保存在`./plot/Tree.png`。
- `mode` : 指定程序的运行模式，可以是 `-koopa` 或 `-riscv` 或 `-perf`。
  - `-koopa` : 将输入的SysY源代码转换成Koopa IR。
  - `-riscv` : 将输入的SysY源代码转换成RISC-V汇编代码。
  - `-perf` : 用于性能测试，通常与 `-riscv` 相同。
- `input_file` : 输入文件路径，应为SysY语言编写的源代码文件。
- `-o output_file` : 指定输出文件的路径。根据 `mode` 的不同，输出文件将是IR或汇编代码。

#### 2.1.2 示例

1. 仅生成Koopa IR，不生成AST图像：
```bash
# 注意compiler的路径，下同
./compiler -koopa example.sy -o example.ir
```
2. 生成RISC-V汇编代码并创建AST图像：
```bash
./compiler -dot -riscv example.sy -o example.s
```
3. 进行性能优化并生成RISC-V：(暂未实现性能优化)
```bash
./compiler -perf example.sy -o performance_result.txt
```

#### 2.1.3 错误处理

- 如果输入文件无法打开，程序会显示错误消息并返回错误代码 `-1`。
- 如果解析输入文件失败，程序同样会显示错误并返回 `-1`。
- 如果输出文件无法创建或写入，程序会显示相应的错误消息并返回 `-1`。

#### 2.1.4 注意

确保在运行程序之前，目录 `./plot/` 已经存在，或者程序有权限创建此目录。否则，生成的AST图像文件将无法保存。


## 3. 测试用例
```c
const int N = 100;
int a[N];

int func(int x) {
    if (x == 1 || x == 0) return 1;
    else return func(x - 1) + func(x - 2);
    while(1){
        return 0;
    
    }
}

int main() {
    int b = 10;
    a[0] = func(b + 2);
}
```
## 4. 输出结果
![Alt text](plot/Tree.png)

