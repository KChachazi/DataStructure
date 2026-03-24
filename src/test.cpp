#include <iostream>
#include "test.h"

// 引入各个模块的测试套件
#include "test/test_myVector.hpp"
#include "test/test_intList.hpp"
#include "test/test_myList.hpp"

int main() {
    std::cout << "==========================================\n";
    std::cout << "   Data Structure Library Test Suite\n";
    std::cout << "==========================================\n";
    
    // 运行注册的所有测试
    int result = TestRegistry::runAllTests();
    
    return result;
}