/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    // 比较运算符（按过期时间排序）
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }

    ~HeapTimer() { clear(); }
    
    /**
     * 调整指定 id 的到期时间
     * @param id 定时器标识
     * @param newExpires 相对时间（毫秒）或新的超时时间（根据实现）
     */
    void adjust(int id, int newExpires);

    /**
     * 添加一个新的定时器节点
     * @param id 定时器 id（通常对应连接 fd）
     * @param timeOut 超时时间（毫秒）
     * @param cb 超时回调函数
     */
    void add(int id, int timeOut, const TimeoutCallBack& cb);

    /**
     * 删除指定 id 的定时器并执行回调（如果存在）
     */
    void doWork(int id);

    /**
     * 清空定时器堆
     */
    void clear();

    /**
     * 周期性检查并触发到期的定时器
     */
    void tick();

    /**
     * 弹出堆顶（最近到期）节点
     */
    void pop();

    /**
     * 获取距离下一个定时器触发还剩多少毫秒
     */
    int GetNextTick();

private:
    void del_(size_t i);
    
    void siftup_(size_t i);

    bool siftdown_(size_t index, size_t n);

    void SwapNode_(size_t i, size_t j);

    std::vector<TimerNode> heap_;

    std::unordered_map<int, size_t> ref_;
};

#endif //HEAP_TIMER_H