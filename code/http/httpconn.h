/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      

#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
public:
    HttpConn();

    ~HttpConn();

    /**
     * 初始化连接
     * @param sockFd 已接受的 socket 文件描述符
     * @param addr 客户端地址
     */
    void init(int sockFd, const sockaddr_in& addr);

    /**
     * 从套接字读取数据到内部读缓冲区
     * @param saveErrno 用于保存 errno 的指针
     * @return 读取字节数，出错返回 -1
     */
    ssize_t read(int* saveErrno);

    /**
     * 将内部写缓冲区的数据写入套接字
     * @param saveErrno 用于保存 errno 的指针
     * @return 写入字节数，出错返回 -1
     */
    ssize_t write(int* saveErrno);

    /**
     * 关闭连接并释放资源
     */
    void Close();

    /**
     * 获取绑定的文件描述符
     */
    int GetFd() const;

    /**
     * 获取远端端口号
     */
    int GetPort() const;

    /**
     * 获取远端 IP 字符串
     */
    const char* GetIP() const;
    
    /**
     * 获取远端地址结构
     */
    sockaddr_in GetAddr() const;
    
    /**
     * 处理请求：解析请求、生成响应并准备发送
     * @return 处理是否成功
     */
    bool process();

    int ToWriteBytes() { 
        return iov_[0].iov_len + iov_[1].iov_len; 
    }

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
    
private:
   
    int fd_;
    struct  sockaddr_in addr_;

    bool isClose_;
    
    int iovCnt_;
    struct iovec iov_[2];
    
    Buffer readBuff_; // 读缓冲区
    Buffer writeBuff_; // 写缓冲区

    HttpRequest request_;
    HttpResponse response_;
};


#endif //HTTP_CONN_H