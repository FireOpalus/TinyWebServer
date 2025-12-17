/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 

#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    /**
     * 可写字节数
     */
    size_t WritableBytes() const;       
    /**
     * 可读字节数
     */
    size_t ReadableBytes() const ;
    /**
     * 已预留但未使用的前置字节数（prepend 区域）
     */
    size_t PrependableBytes() const;

    /**
     * 返回当前可读数据的指针（只读）
     */
    const char* Peek() const;
    /**
     * 确保缓冲区至少能写入 len 字节，必要时进行扩容或移动
     */
    void EnsureWriteable(size_t len);
    /**
     * 标记已写入 len 字节（写指针后移）
     */
    void HasWritten(size_t len);

    /**
     * 从缓冲区检索（丢弃）len 字节
     */
    void Retrieve(size_t len);
    /**
     * 将检索位置移动到 end 指定的位置
     */
    void RetrieveUntil(const char* end);

    /**
     * 检索全部可读数据（重置缓冲区）
     */
    void RetrieveAll() ;
    /**
     * 将全部可读数据作为 std::string 返回并清空缓冲区
     */
    std::string RetrieveAllToStr();

    /**
     * 获取写入位置的常量指针
     */
    const char* BeginWriteConst() const;
    /**
     * 获取写入位置指针
     */
    char* BeginWrite();

    /**
     * 追加指定长度的字符数组到缓冲区
     */
    void Append(const char* str, size_t len);
    /**
     * 追加数据（字符串）到缓冲区
     */
    void Append(const std::string& str);
    /**
     * 追加任意数据到缓冲区
     */
    void Append(const void* data, size_t len);
    /**
     * 追加另一个 Buffer 的全部数据
     */
    void Append(const Buffer& buff);

    /**
     * 从文件描述符读取数据到缓冲区（使用 readv）
     * @param fd 文件描述符
     * @param Errno 用于保存错误码的指针
     * @return 读取字节数，出错返回 -1
     */
    ssize_t ReadFd(int fd, int* Errno);
    /**
     * 将缓冲区数据写入文件描述符
     */
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};

#endif //BUFFER_H