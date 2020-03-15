
#ifndef __JS_EXEC_C_API_H__
#define __JS_EXEC_C_API_H__

extern "C"
{
    void *create_js_client(unsigned int q_size);
    void compile_function(void *client, const char *name, const char *content);
    void exec_function(void *client, const char *func, const char *js_args);
    void destroy_js_client(void *client);
}
#endif
