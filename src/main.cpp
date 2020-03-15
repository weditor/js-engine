
#include "execClient.h"
#include <unistd.h>
#include <iostream>
#include <ctime>

using namespace std;

int main(int args, char **argv)
{
    JsClient client(5);
    using namespace std;
    client.compileFunc("myfunc", R"SCRIPT(
    (item) => { 
        item.data += 1;
        return {'mydata': item.data+1};
    }
    )SCRIPT");
    auto ret = client.execFunc("myfunc", "{\"data\": 9}");
    std::cout << ret << std::endl;
    sleep(1);

    clock_t time_start = clock();
    for (int i = 0; i < 100000; ++i)
    {
        ret = client.execFunc("myfunc", "{\"data\": 9}");
        // std::cout << i << std::endl;
    }
    clock_t time_end = clock();
    cout << "time use:" << 1000 * (time_end - time_start) / (double)CLOCKS_PER_SEC << "ms" << endl;
    return 0;
}
