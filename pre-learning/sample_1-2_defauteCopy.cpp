#include <iostream>
#include <cstdio>
#include <cstring>

class A {
public:
    int val;
    char* name;
    A(int v, const char* n) : val(v) {
        name = new char[strlen(n) + 1];
        strcpy(name, n);
    }
    ~A() {
        delete[] name;
    }
};

int main() {
    A *a1 = new A(10, "Original A1");
    A *a2 = new A(*a1);
    delete a1;

    std::cout << a2->name << '\n';

    delete a2;
    return 0;
}