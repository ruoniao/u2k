# Linux `procfs` 文件系统详解

## 1. 什么是 `procfs`
`/proc` 文件系统（`procfs`）是 Linux 内核提供的一种 **虚拟文件系统**，用于暴露内核状态和系统信息。`procfs` **并不占用磁盘空间**，
而是由内核动态生成，提供了一种通过文件操作来与内核交互的方式。

## 2. `procfs` 的常见用途
### 2.1 读取系统信息
`/proc` 目录下的文件和子目录可以用于查询系统运行状态。例如：
- `/proc/cpuinfo`：CPU 信息
- `/proc/meminfo`：内存使用情况
- `/proc/uptime`：系统运行时间
- `/proc/loadavg`：CPU 负载

示例：
```sh
cat /proc/cpuinfo | grep "model name"
```

### 2.2 读取进程信息
每个运行中的进程在 `/proc` 目录下都有一个对应的目录，名称为 **进程 ID（PID）**。例如：
- `/proc/1234/cmdline`：进程 1234 的命令行参数
- `/proc/1234/status`：进程 1234 的状态信息
- `/proc/1234/fd/`：进程 1234 打开的文件描述符

示例：
```sh
ls -l /proc/$$/fd  # 查看当前 shell 进程的打开文件
```

### 2.3 与内核交互（`/proc/sys`）
`/proc/sys` 目录下的文件可用于 **动态修改内核参数**，而无需重启系统。例如，调整 TCP 连接的最大数量：
```sh
echo 102400 > /proc/sys/net/core/somaxconn
```

## 3. 创建自定义 `procfs` 文件
在某些情况下，开发者需要提供 **自定义接口** 供用户空间访问内核数据，这可以通过 `procfs` 进行。

### 3.1 实现一个 `procfs` 设备
以下是一个简单的 **Linux 内核模块**，创建 `/proc/procfs_demo` 文件，并允许用户空间 **读写数据**。

#### **内核模块 (`procfs_demo.c`)**
```c
```

#### **用户空间测试 (`user_proc_test.c`)**
```c
```

#### **Makefile**
```make
obj-m += procfs_demo.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

### 3.2 测试步骤
1. **编译并加载内核模块**：
    ```sh
    make
    sudo insmod procfs_demo.ko
    ```
2. **检查 `/proc/procfs_demo` 是否存在**：
    ```sh
    ls -l /proc/procfs_demo
    ```
3. **运行用户空间程序**：
    ```sh
    gcc -o user_proc_test user_proc_test.c
    ./user_proc_test
    ```
4. **卸载模块**：
    ```sh
    sudo rmmod procfs_demo
    ```

## 4. `procfs` vs. `sysfs`
| 文件系统 | 主要用途 |
|----------|---------|
| `/proc`  | 提供运行时系统信息和进程状态，可读写 |
| `/sys`   | 主要用于硬件、驱动程序的参数配置，一般只读 |

## 5. 结论
`procfs` 提供了一种方便的方式，让用户空间与内核交互，适用于 **读取系统信息** 和 **实现自定义内核接口**。在现代 Linux 版本中，
部分 `/proc` 功能已经迁移到 `/sys`，但 `procfs` 仍然是系统监控、调试和开发的重要工具。

