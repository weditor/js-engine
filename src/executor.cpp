
#include "executor.h"
#include <thread>

JsExecutor::JsExecutor()
{
    sem_init(&m_sem, 0, 0);
    pthread_mutex_init(&m_func_mutex, nullptr);
    pthread_mutex_init(&m_exec_mutex, nullptr);
}
JsExecutor::~JsExecutor()
{
    pthread_mutex_destroy(&m_exec_mutex);
    pthread_mutex_destroy(&m_func_mutex);
    sem_destroy(&m_sem);
}

void *JsExecutor::start()
{
    auto *thread = new std::thread(&JsExecutor::run, this);
    // thread->detach
    return thread;
}

int JsExecutor::pushCompileFunc(JsFuture<CompileArgs> &args)
{
    MutexGuard guard(&m_func_mutex);
    m_func_queue.push(args);
}
int JsExecutor::pushExecFunc(JsFuture<ExecArgs> &args)
{
    MutexGuard guard(&m_exec_mutex);
    m_exec_queue.push(args);
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
            FutureGuard<CompileArgs> guard(fut);
            m_func_queue.pop();
            fut.m_code = this->compileFunc(fut.m_data.name, fut.m_data.content);
        }
        else if (m_exec_queue.size())
        {
            auto fut = m_exec_queue.front();
            FutureGuard<ExecArgs> guard(fut);
            m_exec_queue.pop();
            fut.m_code = this->execFunc(fut.m_data.name, fut.m_data.jsContent);
        }
    }
}

int JsExecutor::compileFunc(std::string name, std::string content)
{
    return 0;
}
int JsExecutor::execFunc(std::string name, std::string jsArgs)
{
    return 0;
}
