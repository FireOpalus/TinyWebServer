/*
 * @Author       : mark
 * @Date         : 2020-06-18
 * @copyleft Apache 2.0
 */ 
#include <unistd.h>
#include "server/webserver.h"

int main(int argc, char* argv[]) {
    /* 守护进程 后台运行 */
    //daemon(1, 0); 
    bool openLog;
    if(argc < 2) {
       openLog = true;
    }
    else {
       if(strcmp(argv[1], "0") == 0) {
           openLog = false;
       }
       else {
           openLog = true;
       }
    }

    WebServer server(
        1214, 3, 60000, false,             /* 端口 ET模式 timeoutMs 优雅退出  */
        3306, "webserver", "111111", "webserver", /* Mysql配置 */
        12, 6, openLog, 1, 1024);             /* 连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */
    server.Start();
} 
  