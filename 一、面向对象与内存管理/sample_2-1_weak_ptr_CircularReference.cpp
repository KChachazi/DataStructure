#include <iostream>
#include <memory>

class SampleA {
public:
    int data;
    std::shared_ptr<class SampleB> ptrB;
    SampleA(int val, std::shared_ptr<class SampleB> bPtr) : data(val), ptrB(bPtr) { std::cout << "SampleA Constructor" << '\n'; }
    ~SampleA() { std::cout << "SampleA Destructor" << '\n'; }
};

class SampleB {
public:
    int data;
    std::weak_ptr<SampleA> ptrA; // 使用 weak_ptr 防止循环引用
    SampleB(int val, std::shared_ptr<SampleA> aPtr) : data(val), ptrA(aPtr) { std::cout << "SampleB Constructor" << '\n'; }
    ~SampleB() { std::cout << "SampleB Destructor" << '\n'; }
};

int main() {
    {
        std::shared_ptr<SampleA> aPtr = std::make_shared<SampleA>(1, nullptr);
        std::shared_ptr<SampleB> bPtr = std::make_shared<SampleB>(2, aPtr);
        aPtr->ptrB = bPtr;
        std::cout << "SampleA counter: " << aPtr.use_count() << '\n'; // 由于SampleB中使用 weak_ptr ，值为1
        std::cout << "SampleB counter: " << bPtr.use_count() << '\n'; // SampleA 和 SampleB 互相引用，但不会导致内存泄漏
    }

    return 0;
}