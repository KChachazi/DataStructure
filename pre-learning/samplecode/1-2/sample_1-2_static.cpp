#include <iostream>

class A {
public:
    inline static int staticVar = 0;
    static void staticMethod() {
        std::cout << "Static Method called. Static Variable: " << staticVar << std::endl;
    }
};

int main() {
    // 不经过对象直接访问静态成员
    A::staticVar = 10;
    A::staticMethod();
    return 0;
}