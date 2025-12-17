/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/httpconn.h"

class WebServer {
public:
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    ~WebServer();
    void Start();

private:
    /**
     * 初始化监听 socket、绑定端口并开始监听
     */
    bool InitSocket_(); 
    /**
     * 根据触发模式设置事件（LT/ET）
     */
    void InitEventMode_(int trigMode);
    /**
     * 新客户端接入时添加到管理结构并注册定时器
     */
    void AddClient_(int fd, sockaddr_in addr);
  
    /**
     * 处理监听套接字上的可读事件（accept 新连接）
     */
    void DealListen_();
    /**
     * 处理写事件：将待写数据发送到客户端
     */
    void DealWrite_(HttpConn* client);
    /**
     * 处理读事件：读取并解析请求，交给线程池处理
     */
    void DealRead_(HttpConn* client);

    /**
     * 向 fd 发送错误信息并关闭连接
     */
    void SendError_(int fd, const char*info);
    /**
     * 扩展客户端定时器（重置超时时间）
     */
    void ExtentTime_(HttpConn* client);
    /**
     * 关闭客户端连接并清理资源
     */
    void CloseConn_(HttpConn* client);

    /**
     * 读事件的高层回调：从 socket 读取数据并准备处理
     */
    void OnRead_(HttpConn* client);
    /**
     * 写事件的高层回调：处理写完成或继续写入
     */
    void OnWrite_(HttpConn* client);
    /**
     * 处理完成回调：执行请求处理逻辑
     */
    void OnProcess(HttpConn* client);

    static const int MAX_FD = 65536;

    static int SetFdNonblock(int fd);

    int port_;
    bool openLinger_;
    int timeoutMS_;  /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    char* srcDir_;
    
    uint32_t listenEvent_;
    uint32_t connEvent_;
   
    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};


#endif //WEBSERVER_H