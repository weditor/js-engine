#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__
#include <map>
#include <queue>
#include <string>
#include <semaphore.h>
#include <mutex>
#include <iostream>
#include <condition_variable>

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
    JsFuture(T data, std::condition_variable *cond_var, std::mutex *mutex) : m_cond(cond_var), m_mutex(mutex)
    {
        m_data = data;
        m_code = -1;
        m_result = nullptr;
    }
    T m_data;
    int m_code;
    std::string message;
    char *m_result;
    std::mutex *const m_mutex;
    std::condition_variable *m_cond;
};

template <typename T>
class FutureGuard
{
public:
    JsFuture<T> *m_future;
    FutureGuard(JsFuture<T> *future) : m_future(future)
    {
    }
    ~FutureGuard()
    {
        // std::cout << "~FutureGuard() " << m_future << std::endl;
        if (m_future->m_code == -1)
        {
            m_future->m_code = -2;
        }
        m_future->m_cond->notify_one();
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
    void *start();

private:
    void run();
    void init();
    int compileFunc(std::string name, std::string content);
    char *execFunc(std::string name, std::string jsArgs);
    int inner_compile_func(const char *funcText, void **func);

private:
    bool m_ok;
    sem_t m_sem;
    pthread_mutex_t m_func_mutex;
    pthread_mutex_t m_exec_mutex;
    std::queue<JsFuture<CompileArgs> *> m_func_queue;
    std::queue<JsFuture<ExecArgs> *> m_exec_queue;
    std::map<std::string, void *> m_funcMap;

    void *m_runtime;
    void *m_context;
    void *m_wrapperFunc;
};

#endif
