
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
    pthread_cond_t *get_cond_mutex();
    void collect_cond_mutex(pthread_cond_t *mutex);

private:
    const unsigned int m_max_size;
    pthread_mutex_t m_mutex;
    pthread_mutex_t m_vec_mutex;
    std::vector<pthread_cond_t *> m_cond_mutexes;
    JsExecutor m_js_executor;
};

#endif
