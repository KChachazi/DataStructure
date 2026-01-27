#include <iostream>

class Base {
public:
    int base;
    Base(int b) : base(b) {}
    virtual void show() {
        std::cout << "Base class base: " << base << std::endl;
    }
};

class Base1 : public Base {
public:
    int base1;
    Base1(int b, int b1) : Base(b), base1(b1) {}
    void show() override {
        std::cout << "Base1 class base1: " << base1 << std::endl;
    }
};

class Base2 : public Base {
public:
    int base2;
    Base2(int b, int b2) : Base(b), base2(b2) {}
    void show() override {
        std::cout << "Base2 class base2: " << base2 << std::endl;
    }
};

class Derived : public Base1, public Base2 {
public:
    int derived;
    Derived(int b1, int b2, int b, int d) 
        : Base1(b1, b), Base2(b2, b), derived(d) {}
    void show() override {
        std::cout << "Derived class derived: " << derived << std::endl;
    }
};

int main()
{
    Derived obj(1, 2, 3, 4);
    
    std::cout << obj.base << std::endl; // Ambiguous: which base?

    std::cout << obj.Base1::base << std::endl; // Accessing Base1's base
    std::cout << obj.Base2::base << std::endl; // Accessing Base2's base

    return 0;
}