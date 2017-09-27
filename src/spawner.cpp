#include <iostream>
#include "processthread.h"

using namespace std;

int main()
{
    ProcessThread mThread;
    mThread.start();
    mThread.wait(30000);
    return 0;
}
