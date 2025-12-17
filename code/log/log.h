/*
 * @Author       : mark
 * @Date         : 2020-06-16
 * @copyleft Apache 2.0
 */ 
#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         //mkdir
#include "blockqueue.h"
#include "../buffer/buffer.h"

class Log {
public:
    /**
     * 初始化日志系统
     * @param level 日志等级（0-debug,1-info,2-warn,3-error）
     * @param path 日志文件目录
     * @param suffix 日志文件后缀
     * @param maxQueueCapacity 异步队列最大容量
     */
    void init(int level, const char* path = "./log", 
                const char* suffix = ".log",
                int maxQueueCapacity = 1024);

    /**
     * 获取全局单例实例（线程安全，懒汉式）
     * @return Log* 单例指针
     */
    static Log* Instance();

    /**
     * 日志刷新线程入口函数（异步写日志时使用）
     * 由后台线程调用，循环写入队列内容到文件
     */
    static void FlushLogThread();

    /**
     * 写日志，支持 printf 风格参数
     * @param level 日志等级（0-debug,1-info,2-warn,3-error）
     * @param format 格式化字符串
     * @param ... 可变参数
     */
    void write(int level, const char *format,...);

    /**
     * 立即刷新缓冲区内容到磁盘文件
     */
    void flush();

    /**
     * 获取当前日志等级
     * @return int 日志等级
     */
    int GetLevel();

    /**
     * 设置日志等级
     * @param level 日志等级
     */
    void SetLevel(int level);

    /**
     * 判断日志系统是否已打开
     * @return bool 是否打开
     */
    bool IsOpen() { return isOpen_; }
    
private:
    /**
     * 构造函数，初始化成员变量
     */
    Log();

    /**
     * 在日志内容前添加等级标签（如[INFO]）
     * @param level 日志等级
     */
    void AppendLogLevelTitle_(int level);

    /**
     * 析构函数，关闭日志系统，释放资源
     */
    virtual ~Log();

    /**
     * 异步写日志实现函数
     * 从队列取出日志内容并写入文件
     */
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_;
    const char* suffix_;

    int MAX_LINES_;

    int lineCount_;
    int toDay_;

    bool isOpen_;
 
    Buffer buff_;
    int level_;
    bool isAsync_;

    FILE* fp_;
    std::unique_ptr<BlockDeque<std::string>> deque_; 
    std::unique_ptr<std::thread> writeThread_;
    std::mutex mtx_;
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H