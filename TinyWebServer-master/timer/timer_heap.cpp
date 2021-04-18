#include "timer_heap.h"

HeapTimer * HeapTimer::GetInstance()
{
    static HeapTimer timer;
    return &timer;
}

void HeapTimer::siftup_(int i) 
{
    assert(i >= 0 && i < heap.size());
    int j = (i - 1) / 2;
    while(j >= 0 && i != j)
    {
        if(heap[j] < heap[i])
            break;       
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

void HeapTimer::SwapNode_(int i, int j)
{
    assert(i >= 0 && i < heap.size());
    assert(j >= 0 && j < heap.size());
    std::swap(heap[i], heap[j]);
    ref[heap[i].id] = i;
    ref[heap[j].id] = j;
} 

bool HeapTimer::siftdown_(int index, int n) 
{
    assert(index >= 0 && index < heap.size());
    assert(n >= 0 && n <= heap.size());
    int i = index;
    int j = i * 2 + 1;
    while(j < n) 
    {
        if(j + 1 < n && heap[j + 1] < heap[j]) 
            j++;
        if(heap[i] < heap[j]) 
            break;
        SwapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void HeapTimer::add(int id, int timeout, TimeoutCallBack cb)
{
    assert(id >= 0);
    int i;
    if(ref.count(id) == 0) 
    {
        /* 新节点：堆尾插入，调整堆 */
        i = heap.size();
        ref[id] = i;

        timernode_t timer;
        timer.id = id;
        timer.expires = timeout + time(NULL);
        timer.cb_func = cb;
        heap.push_back(timer);
        if(i > 0)
            siftup_(i);
    } 
    else 
    {
        /* 已有结点：调整堆 */
        i = ref[id];
        heap[i].expires = time(NULL) + timeout;
        if(!siftdown_(i, heap.size())) 
        {
            siftup_(i);
        }
    }
}

void HeapTimer::doWork(int id) 
{
    /* 删除指定id结点，并触发回调函数 */
    if(heap.empty() || ref.count(id) == 0) 
    {
        return;
    }
    int i = ref[id];
    timernode_t node = heap[i];
    node.cb_func();
    del(i);
}

void HeapTimer::del(int index) 
{
    /* 删除指定位置的结点 */
    assert(!heap.empty() && index >= 0 && index < heap.size());
    /* 将要删除的结点换到队尾，然后调整堆 */
    int i = index;
    int n = heap.size() - 1;
    assert(i <= n);
    if(i < n) 
    {
        SwapNode_(i, n);
        if(!siftdown_(i, n)) {
            siftup_(i);
        }
    }
    /* 队尾元素删除 */
    ref.erase(heap.back().id);
    heap.pop_back();
}

void HeapTimer::adjust(int id, int timeout) 
{
    /* 调整指定id的结点 */
    assert(!heap.empty() && ref.count(id) > 0);
    heap[ref[id]].expires = time(NULL) + timeout;
    siftdown_(ref[id], heap.size());
}

void HeapTimer::tick() 
{
    /* 清除超时结点 */
    if(heap.empty()) 
    {
        return;
    }

    while(!heap.empty()) 
    {

        timernode_t node = heap.front();

        if( node.expires - time(NULL) > 0) 
        { 
            break; 
        }

        node.cb_func();
        pop();

    }

}

void HeapTimer::pop() 
{
    assert(!heap.empty());
    del(0);
}

void HeapTimer::clear() 
{
    ref.clear();
    heap.clear();
}

