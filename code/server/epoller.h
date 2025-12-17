/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 
#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);

    ~Epoller();

    /**
     * 将 fd 添加到 epoll 实例并设置事件
     * @param fd 文件描述符
     * @param events 监听事件掩码（EPOLLIN | EPOLLOUT 等）
     * @return 是否添加成功
     */
    bool AddFd(int fd, uint32_t events);

    /**
     * 修改已注册 fd 的事件
     */
    bool ModFd(int fd, uint32_t events);

    /**
     * 从 epoll 中删除 fd
     */
    bool DelFd(int fd);

    /**
     * 等待事件发生（封装 epoll_wait）
     * @param timeoutMs 超时时间（毫秒），-1 表示无限等待
     * @return 触发的事件数量
     */
    int Wait(int timeoutMs = -1);

    /**
     * 获取第 i 个事件对应的 fd
     */
    int GetEventFd(size_t i) const;

    /**
     * 获取第 i 个事件的事件标志位
     */
    uint32_t GetEvents(size_t i) const;
        
private:
    int epollFd_;

    std::vector<struct epoll_event> events_;    
};

#endif //EPOLLER_H