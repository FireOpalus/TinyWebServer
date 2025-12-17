/*
 * @Author       : mark
 * @Date         : 2020-06-25
 * @copyleft Apache 2.0
 */ 
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../pool/sqlconnRAII.h"

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,        
    };

    enum HTTP_METHOD {
        GET = 0,
        POST,
        PUT,
        DELETE,
        PATCH,
        HEAD,
        TRACE,
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    
    HttpRequest() { Init(); }
    ~HttpRequest() = default;

    /**
     * 初始化请求解析器，重置内部状态
     */
    void Init();
    /**
     * 从缓冲区解析 HTTP 请求
     * @param buff 包含来自连接的原始数据的缓冲区
     * @return HTTP_CODE 表示解析结果（是否完整、错误等）
     */
    bool parse(Buffer& buff);

    /**
     * 请求路径（只读）
     */
    std::string path() const;
    /**
     * 请求路径（可写引用）
     */
    std::string& path();
    /**
     * HTTP 方法（如 GET/POST）
     */
    std::string method() const;
    /**
     * HTTP 版本字符串（如 HTTP/1.1）
     */
    std::string version() const;
    /**
     * 从解析后的 POST 数据中获取键对应的值（std::string key）
     */
    std::string GetPost(const std::string& key) const;
    /**
     * 从解析后的 POST 数据中获取键对应的值（C 字符串 key）
     */
    std::string GetPost(const char* key) const;

    /**
     * 判断连接是否使用 Keep-Alive
     */
    bool IsKeepAlive() const;

    /* 
    todo 
    void HttpConn::ParseFormData() {}
    void HttpConn::ParseJson() {}
    */

private:
    /**
     * 解析请求行（例如：GET /index.html HTTP/1.1）
     * @return 是否解析成功
     */
    bool ParseRequestLine_(const std::string& line);
    /**
     * 解析单个请求头行（Header: value）
     */
    void ParseHeader_(const std::string& line);
    /**
     * 解析请求体（如 POST 的表单或 JSON）
     */
    void ParseBody_(const std::string& line);

    /**
     * 处理并规范化请求路径（去除 .. 等危险路径）
     */
    void ParsePath_();
    /**
     * 解析 POST 表单数据并填充 post_ 映射
     */
    void ParsePost_();
    /**
     * 解析 application/x-www-form-urlencoded 格式的 POST 数据
     */
    void ParseFromUrlencoded_();

    static bool UserVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PARSE_STATE state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConverHex(char ch);
};


#endif //HTTP_REQUEST_H