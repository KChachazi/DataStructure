#include <iostream>
#include "test.h"

// 引入各个模块的测试套件
#include "test_myVector.hpp"
#include "test_intList.hpp"

int main() {
    std::cout << "==========================================\n";
    std::cout << "   Data Structure Library Test Suite\n";
    std::cout << "==========================================\n";
    
    // 运行注册的所有测试
    int result = TestRegistry::runAllTests();
    
    return result;
}