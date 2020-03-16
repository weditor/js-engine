
#include "execClient.h"

extern "C"
{
    void *cjs_create_js_client(unsigned int q_size)
    {
        return new JsClient(q_size);
    }

    int cjs_compile_function(void *client, const char *name, const char *content)
    {
        JsClient *js_client = (JsClient *)client;
        return js_client->compileFunc(name, content);
    }
    char *cjs_exec_function(void *client, const char *func, const char *js_args)
    {
        JsClient *js_client = (JsClient *)client;
        return js_client->execFunc(func, js_args);
    }
    void cjs_destroy_js_client(void *client)
    {
        JsClient *js_client = (JsClient *)client;
        delete js_client;
    }

    void cjs_free_result(char *result)
    {
        free(result);
    }
}
