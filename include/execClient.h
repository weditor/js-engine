
#ifndef __EXEC_CLIENT_H__
#define __EXEC_CLIENT_H__
#include <pthread.h>
#include <vector>
#include <string>
#include "executor.h"

class JsClient
{
public:
    JsClient(const unsigned int max_size);
    ~JsClient();
    int compileFunc(std::string name, std::string content);
    std::string execFunc(std::string name, std::string content);

private:
    std::condition_variable *get_cond_mutex();
    void collect_cond_mutex(std::condition_variable *mutex);

private:
    const unsigned int m_max_size;
    // pthread_mutex_t m_mutex;
    std::mutex m_mutex;
    pthread_mutex_t m_vec_mutex;
    std::vector<std::condition_variable *> m_cond_mutexes;
    JsExecutor m_js_executor;
};

#endif
