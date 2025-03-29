#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define SYS_hello_world 450  // 确保这里的编号和 `syscall_64.tbl` 一致

int main() {
    long ret = syscall(SYS_hello_world);
    printf("sys_hello_world returned: %ld\n", ret);
    return 0;
}
