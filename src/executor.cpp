
#include "executor.h"

JsExecutor::JsExecutor()
{
    sem_init(&m_sem, 0, 0);
    // pthread_mutex_init(&m_cond_mutex, nullptr);
}
JsExecutor::~JsExecutor()
{
    sem_destroy(&m_sem);
    // pthread_mutex_init(&m_cond_mutex, nullptr);
}
void JsExecutor::run()
{
    for (;;)
    {
        sem_wait(&m_sem);
        if (m_func_queue.size())
        {
            auto fut = m_func_queue.front();
            pthread_mutex_lock(fut.m_mutex);
            FutureGuard(fut);
            m_func_queue.pop();
            fut.m_code = this->compileFunc(fut.m_data.name, fut.m_data.content);
        }
        else if (m_exec_queue.size())
        {
            auto fut = m_exec_queue.front();
            FutureGuard(fut);
            m_exec_queue.pop();
            fut.m_code = this->execFunc(fut.m_data.name, fut.m_data.jsContent);
        }
    }
}