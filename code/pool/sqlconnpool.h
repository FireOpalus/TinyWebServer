/*
 * @Author       : mark
 * @Date         : 2020-06-16
 * @copyleft Apache 2.0
 */ 
#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool {
public:
    static SqlConnPool *Instance();

    /**
     * 从连接池获取一个可用的 MYSQL 连接，可能阻塞直到有连接可用
     * @return MYSQL* 已连接对象
     */
    MYSQL *GetConn();
    /**
     * 释放连接到连接池
     */
    void FreeConn(MYSQL * conn);
    /**
     * 获取当前空闲连接数
     */
    int GetFreeConnCount();

    void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);
    void ClosePool();

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;

    std::queue<MYSQL *> connQue_;
    std::mutex mtx_;
    sem_t semId_;
};


#endif // SQLCONNPOOL_H