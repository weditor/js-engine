
#include "executor.h"

#include "ChakraCore.h"
// #include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
// #include <map>
#include <thread>

#define FAIL_CHECK(cmd)                  \
    do                                   \
    {                                    \
        JsErrorCode errCode = cmd;       \
        if (errCode != JsNoError)        \
        {                                \
            printf("Error %d at '%s'\n", \
                   errCode, #cmd);       \
            return 1;                    \
        }                                \
    } while (0)

using namespace std;

const static char *wrapper_script = R"SCRIPT(
(func, item) => {
    let data = JSON.parse(item);
    let ret = func(data);
    return JSON.stringify(ret);
}
)SCRIPT";

static unsigned currentSourceContext = 0;

JsExecutor::JsExecutor() : m_runtime(nullptr)
{
    sem_init(&m_sem, 0, 0);
    pthread_mutex_init(&m_func_mutex, nullptr);
    pthread_mutex_init(&m_exec_mutex, nullptr);

    m_ok = true;
}
JsExecutor::~JsExecutor()
{
    JsSetCurrentContext(JS_INVALID_REFERENCE);
    JsDisposeRuntime(m_runtime);

    pthread_mutex_destroy(&m_exec_mutex);
    pthread_mutex_destroy(&m_func_mutex);
    sem_destroy(&m_sem);
}

void JsExecutor::init()
{

    JsRuntimeHandle runtime;
    if (JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime) != JsNoError)
    {
        m_ok = false;
        return;
    }
    m_runtime = runtime;

    JsContextRef context;
    // Create an execution context.
    if (JsCreateContext(m_runtime, &context) != JsNoError)
    {
        JsDisposeRuntime(m_runtime);
        m_ok = false;
        return;
    }
    m_context = context;

    // Now set the current execution context.
    if (JsSetCurrentContext(m_context) != JsNoError)
    {
        JsDisposeRuntime(m_runtime);
        m_ok = false;
        return;
    }
    if (inner_compile_func(wrapper_script, &m_wrapperFunc) != 0)
    {
        JsSetCurrentContext(JS_INVALID_REFERENCE);
        JsDisposeRuntime(m_runtime);
        m_ok = false;
        return;
    }
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
    m_func_queue.push(&args);
    sem_post(&m_sem);
}
int JsExecutor::pushExecFunc(JsFuture<ExecArgs> &args)
{
    MutexGuard guard(&m_exec_mutex);
    m_exec_queue.push(&args);
    sem_post(&m_sem);
}

void JsExecutor::run()
{
    init();
    for (;;)
    {
        // std::cout << "start run" << std::endl;
        sem_wait(&m_sem);
        if (m_func_queue.size())
        {
            auto *fut = m_func_queue.front();
            // pthread_mutex_lock(fut.m_mutex);
            FutureGuard<CompileArgs> guard(fut);
            m_func_queue.pop();
            // std::cout << "m_func_queue get ok!" << fut.m_data.name << "|||" << fut.m_data.content << std::endl;
            this->compileFunc(fut->m_data.name, fut->m_data.content);
            fut->m_code = 0;
            // std::cout << "m_func_queue compile ok!" << fut.m_data.name << "|||" << fut.m_data.content << std::endl;
        }
        else if (m_exec_queue.size())
        {
            auto *fut = m_exec_queue.front();
            // std::cout << "m_exec_queue get ok" << std::endl;
            FutureGuard<ExecArgs> guard(fut);
            m_exec_queue.pop();
            char *result = this->execFunc(fut->m_data.name, fut->m_data.jsContent);
            fut->m_result = result;
            fut->m_code = 0;
            // free(result);
        }
    }
}

int JsExecutor::compileFunc(std::string name, std::string content)
{
    if (!m_ok)
    {
        return -1;
    }
    JsValueRef func;
    int ret = this->inner_compile_func(content.c_str(), &func);
    if (ret == 0)
    {
        m_funcMap[name] = func;
    }
    return ret;
    // return 0;
}
char *JsExecutor::execFunc(std::string name, std::string jsArgs)
{
    if (!m_ok)
    {
        return nullptr;
    }
    auto item = m_funcMap.find(name);
    JsValueRef real_func = item->second;
    JsValueRef func_arg, undefined, resultJSString, func_result;

    JsGetUndefinedValue(&undefined);

    if (JsCreateString(jsArgs.c_str(), jsArgs.length(), &func_arg) != JsNoError)
    {
        return nullptr;
    }
    JsValueRef args[] = {undefined, real_func, func_arg};
    int ret = JsCallFunction(m_wrapperFunc, args, 3, &func_result);
    if (ret != JsNoError)
    {
        printf("%x ", ret);
        std::cout << "js call function not ok" << std::endl;
        return nullptr;
    }
    // std::cout << "js call function ok !!!" << std::endl;

    char *resultSTR = nullptr;
    size_t stringLength;

    JsConvertValueToString(func_result, &resultJSString);
    JsCopyString(resultJSString, nullptr, 0, &stringLength);
    resultSTR = (char *)malloc(stringLength + 1);
    JsCopyString(resultJSString, resultSTR, stringLength + 1, nullptr);
    // std::cout << stringLength << " " << resultSTR << std::endl;
    resultSTR[stringLength] = 0;
    return resultSTR;
    // return 0;
}

int JsExecutor::inner_compile_func(const char *funcText, JsValueRef *func)
{
    if (!m_ok)
    {
        std::cout << "js not ok" << std::endl;
        return -1;
    }
    JsValueRef filename;
    int ret = JsCreateString("ch_execution", strlen("ch_execution"), &filename);
    if (ret != JsNoError)
    {
        // printf("%x ", ret);
        std::cout << "js string not ok" << std::endl;
        return -2;
    }
    // std::cout << "js innner string ok" << std::endl;

    JsValueRef scriptSource;
    if (JsCreateExternalArrayBuffer((void *)funcText, (unsigned int)strlen(funcText),
                                    nullptr, nullptr, &scriptSource) != JsNoError)
    {
        std::cout << "js create array args not ok" << std::endl;
        return -3;
    }
    // Run the script.
    ret = JsRun(scriptSource, currentSourceContext++, filename, JsParseScriptAttributeNone, func);
    if (ret != JsNoError)
    {
        printf("%x ", ret);
        std::cout << "JsRun compile not ok" << funcText << std::endl;
        return -4;
    }
    // std::cout << "function <" << funcText << "> create ok" << std::endl;
    // m_funcMap[string(funcName)] = func;
    return 0;
}
