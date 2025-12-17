#include "heaptimer.h"

void HeapTimer::del_(size_t index) {
    assert(!heap_.empty() && index < heap_.size());
    size_t i = index;
    size_t n = heap_.size() - 1;
    if(i < n) {
        SwapNode_(i, n);
        if(!siftdown_(i, n)) {
            siftup_(i);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::SwapNode_(size_t i, size_t j) {
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

bool HeapTimer::siftdown_(size_t index, size_t n) {
    size_t i = index;
    size_t j = 2 * i + 1;
    while(j < n) {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        SwapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void HeapTimer::siftup_(size_t index) {
    size_t i = index;
    size_t j = (i - 1) / 2;
    while(j >= 0) {
        if(heap_[j] < heap_[i]) break;
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

void HeapTimer::adjust(int id, int newExpires) {
    heap_[ref_[id]].expires = Clock::now() + MS(newExpires);
    siftdown_(ref_[id], heap_.size());
}

void HeapTimer::add(int id, int timeOut, const TimeoutCallBack& cb) {
    size_t i;
    if(!ref_.contains(id)) {
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeOut), cb});
        siftup_(i);
    }
    else {
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeOut);
        heap_[i].cb = cb;
        if(!siftdown_(i, heap_.size())) {
            siftup_(i);
        }
    }
}

void HeapTimer::doWork(int id) {
    if(heap_.empty() || !ref_.contains(id)) {
        return;
    }
    size_t i = id;
    TimerNode node = heap_[i];
    node.cb();
    del_(i);
}

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

void HeapTimer::tick() {
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimerNode node = heap_.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break;
        }
        node.cb();
        pop();
    }
}

void HeapTimer::pop() {
    del_(0);
}

int HeapTimer::GetNextTick() {
    tick();
    size_t res = 0;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) res = 0;
    }
    return res;
}