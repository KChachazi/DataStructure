#include <iostream>
#include "test.h"

int main() {
    mydsTest testRunner;
    testRunner.test9_Performance(30000000, 10); // 300万元素，10次运行取平均
    return 0;
}