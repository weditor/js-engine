#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__
#include <queue>
#include <string>
#include <semaphore.h>

struct CompileArgs
{
    std::string name;
    std::string content;
};
struct ExecArgs
{
    std::string name;
    std::string jsContent;
};

template <typename T>
class JsFuture
{
public:
    JsFuture(T data, pthread_cond_t *cond_mutex, pthread_mutex_t *mutex) : m_cond_mutex(cond_mutex), m_mutex(mutex)
    {
        // m_cond_mutex = cond_mutex;
        // m_mutex = mutex;
        m_data = data;
        code = -1;
    }
    T m_data;
    int m_code;
    std::string message;
    std::string m_result;
    pthread_mutex_t *const m_mutex;
    pthread_cond_t *const m_cond_mutex;
};

template <typename T>
class FutureGuard
{
    JsFuture<T> &m_future;
    FutureGuard(JsFuture<T> &future) : m_future(future)
    {
    }
    ~FutureGuard()
    {
        pthread_mutex_lock(m_future->m_mutex);
        pthread_cond_signal(m_future->m_cond_mutex);
        pthread_mutex_unlock(m_future->m_mutex);
    }
};

class MutexGuard
{
public:
    MutexGuard(pthread_mutex_t *mutex) : m_mutex(mutex)
    {
        pthread_mutex_lock(m_mutex);
    }
    ~MutexGuard()
    {
        pthread_mutex_unlock(m_mutex);
    }

private:
    pthread_mutex_t *const m_mutex;
};

class JsExecutor
{
public:
    JsExecutor();
    ~JsExecutor();

    int pushCompileFunc(JsFuture<CompileArgs> &args);
    int pushExecFunc(JsFuture<ExecArgs> &args);
    void start();

private:
    void run();
    int compileFunc(std::string name, std::string content);
    int execFunc(std::string name, std::string jsArgs);

private:
    sem_t m_sem;
    std::queue<JsFuture<CompileArgs>> m_func_queue;
    std::queue<JsFuture<ExecArgs>> m_exec_queue;
};

#endif
