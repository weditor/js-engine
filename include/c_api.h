
#pragma once

extern "C"
{
    void *cjs_create_js_client(unsigned int q_size);
    void cjs_destroy_js_client(void *client);
    int cjs_compile_function(void *client, const char *name, const char *content);
    char *cjs_exec_function(void *client, const char *func, const char *js_args);
    void cjs_free_result(char *result);
}
