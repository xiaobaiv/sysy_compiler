# SysY Compiler 

本实验参考北大编译原理实验指导文档：https://pku-minic.github.io/online-doc/#/ ，目前完成了语法分析和AST的构建，使用graphviz工具进行AST的绘制，后续将会完成IR和目标代码的生成。

## Run

```bash
# 获取项目文件
git clone https://github.com/xiaobaiv/sysy_compiler.git
# 拉取docker image
docker pull guozhengwu/compiler-dev
# 运行容器，挂载项目文件
docker run -it --rm -v <项目文件> guozhengwu/compiler-dev bash
# 在bash中执行以下命令，得到结果
make
build/compiler -koopa /root/compiler/sysy-make-template/debug/hello.c -o /root/compiler/sysy-make-template/debug/hello.koopa

```

## 测试用例

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
## 输出结果
参考/plot/Tree.png


