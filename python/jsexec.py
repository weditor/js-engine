# -*- encoding: utf-8 -*-

import sys
from os.path import join
from pathlib import Path
from ctypes import *  # NOQA


def _get_dll_name():
    if sys.platform == "win32":
        return 'libchakra_exec.dll'
    else:
        return 'libchakra_exec.so'


js_lib = cdll.LoadLibrary(Path(__file__).parent /'lib'/ _get_dll_name())

cjs_create_executor = js_lib.cjs_create_js_client
cjs_create_executor.argtypes = [c_uint]
cjs_create_executor.restype = c_void_p

cjs_destroy_executor = js_lib.cjs_destroy_js_client
cjs_destroy_executor.argtypes = [c_void_p]

cjs_compile_func = js_lib.cjs_compile_function
cjs_compile_func.argtypes = [c_void_p, c_char_p, c_char_p]
cjs_compile_func.restype = c_int

cjs_run_func = js_lib.cjs_exec_function
cjs_run_func.argtypes = [c_void_p, c_char_p, c_char_p]
cjs_run_func.restype = POINTER(c_char)

cjs_free_result = js_lib.cjs_free_result
cjs_free_result.argtypes = [POINTER(c_char)]

if __name__ == "__main__":
    import json
    import threading
    def mytest():
        print(threading.get_ident())
        ret = cjs_run_func(js_executor, b"myfunc", json.dumps(data, ensure_ascii=False).encode())
        if (ret):
            ret_str = cast(ret, c_char_p)
            print(threading.get_ident(), ret_str.value.decode("utf-8"))

    data = {
        "document": [
            {"block_id": "0", "text": "天行健，君子以自强不息"},
            {"block_id": "1", "text": "地势坤，君子以厚德载物"},
        ]
    }
    js_executor = cjs_create_executor(10)
    cjs_compile_func(js_executor, b"myfunc", open('test.js', 'rb').read())
    t1 = threading.Thread(target=mytest)
    t2 = threading.Thread(target=mytest)
    t3 = threading.Thread(target=mytest)
    t1.start()
    t2.start()
    t3.start()
    ret = cjs_run_func(js_executor, b"myfunc", json.dumps(data, ensure_ascii=False).encode())
    if (ret):
        ret_str = cast(ret, c_char_p)
        print(ret_str.value.decode("utf-8"))

    t1.join()
    t2.join()
    t3.join()
    cjs_free_result(ret)
    cjs_destroy_executor(js_executor)
