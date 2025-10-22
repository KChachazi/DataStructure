#include <iostream>
#include <cstdio>
#include <cstring>

class A {
public:
    int val;
    char *name;
    A(int v, const char *n) : val(v) {
        name = new char(strlen(n) + 1);
        strcpy(name, n);
    }

    // 深拷贝构造函数
    A(const A &other) : val(other.val) {
        name = new char(strlen(other.name) + 1);
        strcpy(name, other.name);
    }

    // 深拷贝赋值运算符重载
    A& operator =(const A &other) {
        if (this == &other) return *this;

        delete[] name; // 释放已有资源
        val = other.val;
        name = new char(strlen(other.name) + 1);
        strcpy(name, other.name);
        return *this; // 返回自身以支持链式赋值 a = b = c;
    }

    // 析构函数
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