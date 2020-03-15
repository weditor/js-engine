
#include "execClient.h"

void *create_js_client(unsigned int q_size)
{
    return new JsClient(q_size);
}

void compile_function(void *client, const char *name, const char *content)
{
    JsClient *js_client = (JsClient *)client;
    js_client->compileFunc(name, content);
}
void exec_function(void *client, const char *func, const char *js_args)
{
    JsClient *js_client = (JsClient *)client;
    js_client->execFunc(func, js_args);
}
void destroy_js_client(void *client)
{
    JsClient *js_client = (JsClient *)client;
    delete js_client;
}
