#include "execClient.h"

JsClient::JsClient(const unsigned int max_size) : m_max_size(max_size)
{
    // pthread_mutex_init(&m_mutex, nullptr);
    pthread_mutex_init(&m_vec_mutex, nullptr);
    for (unsigned int i = 0; i < max_size; ++i)
    {
        std::condition_variable *mutex = new std::condition_variable;
        // pthread_cond_init(mutex, nullptr);
        m_cond_mutexes.push_back(mutex);
    }
    m_js_executor.start();
}
JsClient::~JsClient()
{
    // pthread_mutex_destroy(&m_mutex);
    pthread_mutex_destroy(&m_vec_mutex);
    for (auto *mutex : m_cond_mutexes)
    {
        // pthread_cond_destroy(mutex);
        delete mutex;
    }
}

int JsClient::compileFunc(std::string name, std::string content)
{
    // 修改为构造函数保证 cond_mutex 回收。
    auto *cond_mutex = get_cond_mutex();
    std::function<void()> myfunc = [&]() { this->collect_cond_mutex(cond_mutex); };
    FunctionGuard guard(myfunc);

    auto fut = JsFuture<CompileArgs>({name, content}, cond_mutex, &m_mutex);
    std::unique_lock<std::mutex> lock(m_mutex);
    m_js_executor.pushCompileFunc(fut);
    cond_mutex->wait(lock, [&]() { return fut.m_code != -1; });

    // collect_cond_mutex(cond_mutex);
    return fut.m_code;
}
char *JsClient::execFunc(std::string name, std::string content)
{
    // 修改为构造函数保证 cond_mutex 回收。
    auto *cond_mutex = get_cond_mutex();
    std::function<void()> myfunc = [&]() { this->collect_cond_mutex(cond_mutex); };
    FunctionGuard guard(myfunc);

    auto fut = JsFuture<ExecArgs>({name, content}, cond_mutex, &m_mutex);
    std::unique_lock<std::mutex> lock(m_mutex);
    m_js_executor.pushExecFunc(fut);
    cond_mutex->wait(lock, [&]() { return fut.m_code != -1; });

    // collect_cond_mutex(cond_mutex);
    return fut.m_result;
}

std::condition_variable *JsClient::get_cond_mutex()
{
    MutexGuard guard(&m_vec_mutex);
    std::condition_variable *mutex = nullptr;
    if (m_cond_mutexes.size() > 0)
    {
        mutex = m_cond_mutexes.back();
        m_cond_mutexes.pop_back();
    }
    return mutex;
}

void JsClient::collect_cond_mutex(std::condition_variable *mutex)
{
    MutexGuard guard(&m_vec_mutex);
    m_cond_mutexes.push_back(mutex);
}

FunctionGuard::FunctionGuard(std::function<void()> &func) : m_func(func)
{
}

FunctionGuard::~FunctionGuard()
{
    m_func();
}