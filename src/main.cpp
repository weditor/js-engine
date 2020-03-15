
#include "execClient.h"
#include <iostream>

using namespace std;

int main(int args, char **argv)
{
    JsClient client(5);
    client.compileFunc("myfunc", R"SCRIPT(
    (func, item) => { 
        let data = JSON.parse(item);
        let ret = func(data);
        return JSON.stringify(ret);
    }
    )SCRIPT");
    return 0;
}
