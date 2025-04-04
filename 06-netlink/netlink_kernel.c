#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/string.h>
#include <net/sock.h>
#include <net/net_namespace.h> // 需要包含这个头文件来使用 init_net

#define NETLINK_USER 31  // 定义 Netlink 用户组号

struct sock *nl_sk = NULL;

// Netlink 消息处理函数
static void netlink_recv_msg(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    char *recv_msg;
    int num, new_num;
    char send_msg[32];
    struct sk_buff *skb_out;
    int msg_size;
    int pid;
    int res;

    nlh = nlmsg_hdr(skb);
    recv_msg = (char *)NLMSG_DATA(nlh);
    pr_info("Kernel received: %s\n", recv_msg);

    // 字符串转换为整数
    if (kstrtoint(recv_msg, 10, &num)) {
        pr_err("Invalid number format\n");
        return;
    }

    new_num = num + 1;  // 增加 1
    snprintf(send_msg, sizeof(send_msg), "%d", new_num);  // 将新数字转换为字符串

    msg_size = strlen(send_msg) + 1;
    pid = nlh->nlmsg_pid;  // 获取发送进程的 PID

    skb_out = nlmsg_new(msg_size, GFP_KERNEL);
    if (!skb_out) {
        pr_err("Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    strncpy(NLMSG_DATA(nlh), send_msg, msg_size);  // 复制数据到消息中

    // 发送数据到用户态
    res = netlink_unicast(nl_sk, skb_out, pid, MSG_DONTWAIT);
    if (res < 0) {
        pr_err("Failed to send response\n");
    }
}

// 初始化 Netlink
static int __init netlink_init(void) {
    struct netlink_kernel_cfg cfg = {
            .input = netlink_recv_msg,  // 注册消息接收处理函数
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        pr_err("Failed to create Netlink socket\n");
        return -ENOMEM;
    }

    pr_info("Netlink kernel module loaded\n");
    return 0;
}

// 卸载 Netlink
static void __exit netlink_exit(void) {
    netlink_kernel_release(nl_sk);
    pr_info("Netlink kernel module unloaded\n");
}

module_init(netlink_init);
module_exit(netlink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ruoniao");
MODULE_DESCRIPTION("Netlink Kernel to Userspace Example");
