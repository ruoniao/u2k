```markdown
# Linux 内核自带的系统调用示例 —— `getpid`

Linux 提供了一系列系统调用，其中 `getpid` 用于获取当前进程的 PID（进程 ID）。

## 1. `getpid` 系统调用的内核实现

在 `kernel/sys.c` 文件中，`getpid` 系统调用的实现如下：

```c
#include <linux/syscalls.h>
#include <linux/sched.h>

SYSCALL_DEFINE0(getpid) {
    return task_tgid_vnr(current);
}
```

### **代码解析**
- `SYSCALL_DEFINE0(getpid)`: 定义一个无参数的系统调用。
- `task_tgid_vnr(current)`: 返回当前进程的 TGID（线程组 ID），对于单线程进程，TGID 就是 PID。
- `current`: 指向当前进程的 `task_struct` 结构体。

## 2. `getpid` 在 `syscall_64.tbl` 中的注册

在 `arch/x86/entry/syscalls/syscall_64.tbl` 中，`getpid` 的定义如下：

```plaintext
39   common   sys_getpid
```

其中：
- `39` 是 `getpid` 在 x86_64 架构上的系统调用号。
- `common` 表示它适用于所有 64 位架构。
- `sys_getpid` 是内核中的函数名称。

## 3. `getpid` 在 `syscalls.h` 中的声明

在 `include/linux/syscalls.h` 末尾可以找到：

```c
asmlinkage long sys_getpid(void);
```

## 4. 在用户空间调用 `getpid`

用户态程序可以使用 `syscall(SYS_getpid)` 或 `getpid()` 来调用该系统调用。

### **示例代码**

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main() {
    printf("PID using getpid(): %d\n", getpid());
    printf("PID using syscall: %ld\n", syscall(SYS_getpid));
    return 0;
}
```

### **编译并运行**

```sh
gcc test_getpid.c -o test_getpid
./test_getpid
```

### **示例输出**

```plaintext
PID using getpid(): 1234
PID using syscall: 1234
```

这说明 `getpid()` 直接调用了 `sys_getpid`，并返回当前进程的 PID。

## 5. `dmesg` 查看内核日志

如果在 `getpid` 系统调用中加入 `printk` 进行调试，如：

```c
SYSCALL_DEFINE0(getpid) {
    printk(KERN_INFO "getpid called by process: %d\n", task_tgid_vnr(current));
    return task_tgid_vnr(current);
}
```

然后重新编译内核并运行 `./test_getpid`，可以通过 `dmesg` 查看日志：

```sh
dmesg | tail -n 10
```

示例输出：

```plaintext
[ 5678.123456] getpid called by process: 1234
```

这样，我们就成功分析了 Linux 内核自带的 `getpid` 系统调用的实现、注册过程，以及如何在用户态调用它。

