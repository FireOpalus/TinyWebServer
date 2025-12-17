# server 模块

服务器主控逻辑，包括 epoll 封装、主循环、连接管理等。

- `epoller.*`：epoll 封装，负责高效事件通知。
- `webserver.*`：主服务器类，实现主循环、连接分发、资源管理等。
