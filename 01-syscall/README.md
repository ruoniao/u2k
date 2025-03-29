```markdown
# 在 Linux 内核中新增 `hello_world` 系统调用

## 1. 添加 `hello_world` 系统调用代码

在 `kernel/hello_syscall.c` 文件中添加以下内容：

```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(hello_world) {
    printk(KERN_INFO "Hello, World! from Kernel\n");
    return 42;  // 返回一个数，表示调用成功
}
```

## 2. 修改 `Makefile`

在 `kernel/Makefile` 添加：

```make
obj-y += hello_syscall.o
```

## 3. 注册系统调用号

在 `arch/x86/entry/syscalls/syscall_64.tbl` 添加：

```plaintext
450   common   sys_hello_world
```

## 4. 声明系统调用

在 `include/linux/syscalls.h` 末尾添加：

```c
asmlinkage long sys_hello_world(void);
```
asmlinkage 是 Linux 内核中的一个关键字，它的作用是确保系统调用的参数按照 C 语言调用约定 (C calling convention) 传递，
而不是按照 x86_64 默认的 fastcall 约定 (register calling convention)。 主要作用 在 x86_64 架构上，普通的函数调用通常使用寄存器
传递参数，而 asmlinkage 修饰的函数则强制从 栈 读取参数。这在系统调用处理过程中至关重要，因为用户态调用内核函数时，参数必须按照 统一的调用约定
传递，以确保正确解析用户传递的参数

## 5. 重新编译内核

```sh
make -j$(nproc)
run.sh # qemu重新启动新内核
```


## 7. 在qemu里的用户空间调用 `sys_hello_world`

创建 `hello_syscall.c`：

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_hello_world 450  // 确保这里的编号和 `syscall_64.tbl` 一致

int main() {
    long ret = syscall(SYS_hello_world);
    printf("sys_hello_world returned: %ld\n", ret);
    return 0;
}
```

编译并运行：

```sh
gcc hello_syscall.c -o hello_syscall
./hello_syscall
```

然后检查 `dmesg` 输出：

```sh
dmesg | tail -n 10
```

如果成功，会看到：

```plaintext
[ 1234.567890] Hello, World! from Kernel
```
至此，DEMO完成
