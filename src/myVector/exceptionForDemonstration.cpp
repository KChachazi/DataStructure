#include <iostream>
#include <string>
#include <stdexcept>
#include <exception>

void checkAge(int age) {
    if (age < 0 || age > 150) {
        throw std::out_of_range("年龄必须在0~150之间");
    }
    std::cout << "年龄有效: " << age << std::endl;
}

int main() {
    std::cout << "请输入年龄: ";
    int age;
    std::cin >> age;
    try {
        checkAge(age);  // 抛出异常
    }
    catch (const std::out_of_range& e) {
        std::cerr << "输入错误: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "其他标准异常: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "未知异常" << std::endl;
    }
    return 0;
}