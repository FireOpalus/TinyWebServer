/*
 * @Author       : mark
 * @Date         : 2020-06-16
 * @copyleft Apache 2.0
 */ 
#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

template<class T>
class BlockDeque {
public:
    explicit BlockDeque(size_t MaxCapacity = 1000);

    ~BlockDeque();

    /**
     * 清空队列
     */
    void clear();

    /**
     * 判断队列是否为空
     */
    bool empty();

    /**
     * 判断队列是否已满
     */
    bool full();

    /**
     * 关闭队列并唤醒所有阻塞线程
     */
    void Close();

    /**
     * 返回当前队列大小
     */
    size_t size();

    /**
     * 返回队列容量
     */
    size_t capacity();

    /**
     * 获取队头元素（不删除）
     */
    T front();

    /**
     * 获取队尾元素（不删除）
     */
    T back();

    /**
     * 从队尾压入元素（可能阻塞直到有空间）
     */
    void push_back(const T &item);

    /**
     * 从队头压入元素（可能阻塞直到有空间）
     */
    void push_front(const T &item);

    /**
     * 弹出队头元素，阻塞直到有数据或队列关闭
     * @return 是否成功弹出
     */
    bool pop(T &item);

    /**
     * 带超时弹出队头元素，超时返回 false
     */
    bool pop(T &item, int timeout);

    /**
     * 唤醒一个消费者（通常用于结束阻塞）
     */
    void flush();

private:
    std::deque<T> deq_;

    size_t capacity_;

    std::mutex mtx_;

    bool isClose_;

    std::condition_variable condConsumer_;

    std::condition_variable condProducer_;
};

template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity):capacity_(MaxCapacity) {
    assert(MaxCapacity > 0);
    isClose_ = false;
}

template<class T>
BlockDeque<T>::~BlockDeque() {
    Close();
}

template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
}

template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}

template<class T>
bool BlockDeque<T>::full() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

template<class T>
void BlockDeque<T>::Close() {
    {
        std::lock_guard<std::mutex> locker(mtx_);
        deq_.clear();
        isClose_ = true;
    }
    condConsumer_.notify_all();
    condProducer_.notify_all();
}

template<class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

template<class T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

template<class T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_) {
        condConsumer_.wait(locker);
    }
    deq_.push_back(item);
    condConsumer_.notify_one();
}

template<class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_) {
        condProducer_.wait(locker);
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}

template<class T>
bool BlockDeque<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty()) {
        condConsumer_.wait(locker);
        if(isClose_) return false;
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty()) {
        if(condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout)
            return false;
        if(isClose_) return false;
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<class T>
void BlockDeque<T>::flush() {
    condConsumer_.notify_one();
}

#endif // BLOCKQUEUE_H