/*
 * @Author       : mark
 * @Date         : 2020-06-25
 * @copyleft Apache 2.0
 */ 
#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    /**
     * 初始化响应对象
     * @param srcDir 资源根目录
     * @param path 请求资源路径（相对 srcDir）
     * @param isKeepAlive 是否保持长连接
     * @param code HTTP 状态码（-1 表示正常处理）
     */
    void Init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    /**
     * 根据当前响应状态构建 HTTP 响应并写入缓冲区
     */
    void MakeResponse(Buffer& buff);
    /**
     * 解除 mmap 映射（如果有映射文件）
     */
    void UnmapFile();
    /**
     * 返回映射的文件地址
     */
    char* File();
    /**
     * 返回映射文件长度
     */
    size_t FileLen() const;
    /**
     * 构造错误内容并写入缓冲区
     */
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return code_; }

private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);

    void ErrorHtml_();
    std::string GetFileType_();

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;
    
    char* mmFile_; 
    struct stat mmFileStat_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};


#endif //HTTP_RESPONSE_H