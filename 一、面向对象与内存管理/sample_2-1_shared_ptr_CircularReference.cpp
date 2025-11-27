#include <iostream>
#include <memory>

class SampleA {
public:
    std::shared_ptr<class SampleB> b_ptr;
    SampleA() { std::cout << "SampleA Constructor" << '\n'; }
    ~SampleA() { std::cout << "SampleA Destructor" << '\n'; }
};

class SampleB {
public:
    std::shared_ptr<SampleA> a_ptr;
    SampleB() { std::cout << "SampleB Constructor" << '\n'; }
    ~SampleB() { std::cout << "SampleB Destructor" << '\n'; }
};

int main() {
    std::shared_ptr<SampleA> a = std::make_shared<SampleA>();
    std::shared_ptr<SampleB> b = std::make_shared<SampleB>();
    a->b_ptr = b;
    b->a_ptr = a;
    std::cout << "Reference Count of SampleA: " << a.use_count() << '\n';
    std::cout << "Reference Count of SampleB: " << b.use_count() << '\n';
    return 0;
}