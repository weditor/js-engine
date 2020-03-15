#include "execClient.h"

JsClient::JsClient(const unsigned int max_size) : m_max_size(max_size)
{
    pthread_mutex_init(&m_mutex, nullptr);
    pthread_mutex_init(&m_vec_mutex, nullptr);
    for (unsigned int i = 0; i < max_size; ++i)
    {
        pthread_cond_t *mutex = new pthread_cond_t;
        pthread_cond_init(mutex, nullptr);
        m_cond_mutexes.push_back(mutex);
    }
    m_js_executor.start();
}
JsClient::~JsClient()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_mutex_destroy(&m_vec_mutex);
    for (auto *mutex : m_cond_mutexes)
    {
        pthread_cond_destroy(mutex);
        delete mutex;
    }
}

int JsClient::compileFunc(std::string name, std::string content)
{
    // 修改为构造函数保证 cond_mutex 回收。
    pthread_cond_t *cond_mutex = get_cond_mutex();

    auto fut = JsFuture<CompileArgs>({name, content}, cond_mutex, &m_mutex);
    pthread_mutex_lock(fut.m_mutex);
    m_js_executor.pushCompileFunc(fut);
    pthread_cond_wait(fut.m_cond_mutex, fut.m_mutex);
    pthread_mutex_unlock(fut.m_mutex);

    collect_cond_mutex(cond_mutex);
    return fut.m_code;
}
std::string JsClient::execFunc(std::string name, std::string content)
{
    // 修改为构造函数保证 cond_mutex 回收。
    pthread_cond_t *cond_mutex = get_cond_mutex();

    auto fut = JsFuture<ExecArgs>({name, content}, cond_mutex, &m_mutex);
    pthread_mutex_lock(fut.m_mutex);
    m_js_executor.pushExecFunc(fut);
    pthread_cond_wait(fut.m_cond_mutex, fut.m_mutex);
    pthread_mutex_unlock(fut.m_mutex);

    collect_cond_mutex(cond_mutex);
    return fut.m_result;
}

pthread_cond_t *JsClient::get_cond_mutex()
{
    MutexGuard guard(&m_vec_mutex);
    pthread_cond_t *mutex = nullptr;
    if (m_cond_mutexes.size() > 0)
    {
        mutex = m_cond_mutexes.back();
        m_cond_mutexes.pop_back();
    }
    return mutex;
}

void JsClient::collect_cond_mutex(pthread_cond_t *mutex)
{
    MutexGuard guard(&m_vec_mutex);
    m_cond_mutexes.push_back(mutex);
}