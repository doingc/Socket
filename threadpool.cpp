#include "threadpool.h"

template <class T>
threadpool<T>::threadpool(int thread_num,int max_request):
        _thread_num(thread_num),_max_request(max_request),
        _over(false),_threads(NULL) {
    //线程数量和最大请求量不能为0
    if(_thread_num == 0||_max_request == 0)
        throw exception();

    //初始化子线程数组
    _threads = new pthread_t[thread_num];
    if(!_threads)
        throw exception();
    
    //创建子线程
    for(int i = 0;i<_thread_num;++i){
        if(pthread_create(_threads+i,NULL,worker,this)!=0){
            delete [] _threads;
            throw exception();
        }

        if(pthread_detach(_threads[i])!=0){
            delete [] _threads;
            throw exception();
        }
    }
}

template <class T>
threadpool<T>::~threadpool(){
    delete [] _threads;
    _over = true;
}

//主线程操作请求队列（添加）
template <class T>
bool threadpool<T>::append(T *request){ 
    if(_workQueue.size()>=_max_request){
        return false;
    }

    //主线程添加请求队列，此时其他线程不能操作队列 
    _queueLocker.lock();

    _workQueue.push_back(request);

    _queueStat.post();//通知子线程来任务了
    _queueLocker.unlock();
    return true;
}

template <class T>
void* threadpool<T>::worker(void *arg){
    threadpool *pool = (threadpool *)arg;
    //调用run
    pool->run();
    return pool;
}

//子线程操作请求队列（获取）,并工作
template <class T>
void threadpool<T>::run(){
    while (!m_stop){
        _queueStat.wait();//工作队列为空就等待挂起，工作队列不为空才能获取任务
        _queueLocker.lock();//获取任务时，要使用互斥锁，保证对资源的独占式访问

        T *request = _workQueue.front();//获取任务
        _workQueue.pop_front();

        _queueLocker.unlock();

        if(!request){
            continue;
        }
        //调用任务的工作 逻辑函数
        request->process();//执行任务（工作），这里不用锁，并发执行
    }
   
}
