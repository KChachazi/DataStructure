#include <iostream>
#include <cstdio>

class A {
public:
    virtual void functionA() {
        std::cout << "This is A::functionA()" << '\n';
    }
};

class B {
public:
    virtual void functionB() {
        std::cout << "This is B::functionB()" << '\n';
    }
};

class C : public A, public B {
public:
    void functionA() override {
        std::cout << "This is C::functionA()" << '\n';
    }
    void functionB() override {
        std::cout << "This is C::functionB()" << '\n';
    }
};

int main() {
    A a;
    B b;
    C c;
    std::cout << "Size of class A: " << sizeof(a) << '\n';
    std::cout << "Size of class B: " << sizeof(b) << '\n';
    std::cout << "Size of class C: " << sizeof(c) << '\n';
    return 0;
}