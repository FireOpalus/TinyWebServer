# WebServer
用C++实现的高性能WEB服务器，经过webbenchh压力测试可以实现上万的QPS

## 功能
* 利用IO复用技术Epoll与线程池实现多Reactor高并发模型；
* 实现自动增长的缓冲区；
* 基于小根堆实现的定时器，关闭超时的非活动连接；
* 利用单例模式与阻塞队列实现异步的日志系统，记录服务器运行状态；
* 利用正则与状态机解析HTTP请求报文，实现处理静态资源的请求；

## 环境要求
* Linux
* C++14
* MySql

## 致谢
Linux高性能服务器编程，游双著.

[@qinguoyi](https://github.com/qinguoyi/TinyWebServer)

基于C++14优雅实现的Webserver

[@markparticle](https://github.com/markparticle/WebServer)
