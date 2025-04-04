#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024  // Buffer size

int main() {
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct msghdr msg;
    int sock_fd;
    char send_buf[32];

    // 创建 Netlink socket
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();  // 本进程的 PID
    src_addr.nl_groups = 0;      // 单播

    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("bind");
        close(sock_fd);
        return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;  // 发送到内核
    dest_addr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    // 发送数据
    snprintf(send_buf, sizeof(send_buf), "123");  // 发送整数 123
    strcpy(NLMSG_DATA(nlh), send_buf);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Sending message to kernel: %s\n", send_buf);
    if (sendmsg(sock_fd, &msg, 0) < 0) {
        perror("sendmsg failed");
        free(nlh);
        close(sock_fd);
        return -1;
    }

    // 接收来自内核的消息
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    if (recvmsg(sock_fd, &msg, 0) < 0) {
        perror("recvmsg failed");
        free(nlh);
        close(sock_fd);
        return -1;
    }

    printf("Received from kernel: %s\n", (char *)NLMSG_DATA(nlh));

    free(nlh);
    close(sock_fd);

    return 0;
}
