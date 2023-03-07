#pragma once
#include <list>
#include <pthread.h>
#include <iostream>
#include "locker.h"
using namespace std;

template <class T>//方便重用，T表示任务
class threadpool
{
private:
    //子线程的工作函数，必须是静态成员函数，因为worker只能接收
    //一个参数，但是在类里面有隐藏参数this
    static void *worker(void *arg);
    //
    void run();
private:
    //线程的数量
    int _thread_num;

    //放置线程的数组，简易实现线程池
    pthread_t *_threads;

    //请求队列中最大的请求数量
    int _max_request;

    //请求队列
    list<T*> _workQueue;

    //保护请求队列的互斥锁，因为线程们（包括主线程）共享这个请求队列，从这个队列取任务，所以要线程同步，
    //保证线程的独占式访问
    locker _queueLocker;//调用无参构造函数，初始化

    //信号量，是否有任务需要处理
    semer _queueStat;//调用无参构造函数，初始化

    //是否结束主线程
    bool _over;
public:
    threadpool(int thread_num = 8,int max_request = 1000);
    ~threadpool();
    //主线程往队列中添加任务
    bool append(T *request);

};

