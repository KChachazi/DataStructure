# Updates

续```part1-2.md```继续写

- 2025/11/4: 完成part1-3**C++对象模型**部分。

前面文章涉及了更加高级的面向对象的细节和行为控制，主要包含深/浅拷贝、运算符重载、友元机制、静态成员、常量成员。

接下来将结束面向对象部分，介绍**C++对象模型**，从思想的角度试图对C++的面向对象思想做出阐释。

# 面向对象与内存管理

## **C++对象模型**

### 前言

笔者选择性阅读了许多相关资料，也针对其中的问题向某些大模型进行提问。起初笔者认为**C++对象模型**只是对类与对象机制的再总结，但在进一步阅读与思考后发现，它的本质是一个关于“语言语义如何落实为内存结构与行为机制”的模型。

基于笔者的思考过程，本节的小标题会与前文不同，以一个个问题形式引出。这也是因为**C++对象模型**是一种“设计方案”，从设计初衷到解决问题到最终实现的过程比单纯分析最终设计重要得多。

下面的内容也许带有一定的主观性，请批判性阅读。

### 什么是**C++对象模型**？

**C++对象模型**是语言抽象到内存现实的映射规则。

它的核心内容/本质是定义了，编译器如何将类、继承、多态等高级语义实现为底层的内存布局、函数调用与对象生命周期管理机制。

所以，**C++对象模型**统一但并不单一，实际上不同编译器的具体实现在语义层面等价，在内存布局、虚函数表、跨平台兼容层面上各有不同。各个编译器最终都要实现完整的**C++对象模型**，其面对的主要问题是一致的，只是在实现(作答)上有所不同。

### **C++对象模型**主要在回答的四个问题

#### 对象的内存结构是怎样的？

一个C++对象的本质是一段连续的内存块。编译器要完成的任务是，根据类的定义规则完成这段内存中内存地址与成员的映射。注意，这里的成员指的是非静态成员与内部元信息(如`vptr`)。

##### 对于普通类

```cpp
class Point {
    int x;
    int y;
};
```

对于如上`Point`类生成的对象，编译器会将`x, y`分别顺序放入连续的内存空间(可能对齐填充)。

##### 对于单继承类

```cpp
class Base {
    int a;
};

class Derived : public Base {
    int b;
};
```

派生类对象包含基类对象，且基类对象位于内存前端。即`Derived`类对象地址 `==` `Base`基类子对象地址。

##### 对于多继承类

```cpp
class Base1 {int a; };
class Base2 {int b; };
class Derived : public Base1, public Base2 {
    int c;
};
```

与单继承类相似，派生类对象包含基类对象，且基类对象位于内存前端。两个基类子对象互相独立。

##### 含虚函数类

```cpp
class Base{
    int x;
    virtual void f();
};
```

虚函数会使对象多一个隐藏成员`vptr`，该成员为一个指针，指向该类的虚函数表。

#### 成员函数的调用如何实现？

成员函数本质上只是普通函数，在调用时自动传入`this`指针作为隐式参数。

```cpp
class Counter{
    int val;
public:
    void add(int x) {
        val += x;
    }
};
Counter counter;

// 调用 counter.add(c)
// 相当于
// 调用 Counter_add(&counter, c)
```

对于`this`指针类型有如下讨论：
- 在普通成员函数中为：`Classname* const this`
- 在const成员函数中为：`const Classname* const this`

访问成员时与一般指针相同，都是在计算偏移。

#### 多态和vtable如何工作？

C++对象模型中的多态指的是运行时多态，在运行时确定该变量的类型后才会决定使用哪一种函数。支持这种运行时多态的基本语法是虚函数。当类中存在虚函数时，编译器会自动为类生成：

- `vtable`(虚函数表)：存储虚函数地址
- `vptr`(虚表指针)：**每个对象**包含一个隐式变量指针指向该类的`vtable`

##### 虚函数表行为：动态绑定机制

其行为可以由下面的代码展示：

```cpp
#include <iostream>
class Base {
public:
    virtual void speak() const { std::cout << "Base speaking\n"; }
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    void speak() const override { std::cout << "Derived speaking\n"; }
};

int main() {
    Derived d;
    Base* b = &d;
    b->speak(); // Derived speaking
}
```

在上面的实例中，当通过 `Base*` 调用 `speak()` 时，运行时通过 `vptr` 查表找到 `Derived::speak()`。如果没有虚函数，则静态绑定：`Base::speak()`。

##### 构造与析构中的虚表绑定

在构造和析构阶段，对象的类型尚未完全建立（或已部分销毁），因此：**在构造/析构中调用虚函数不会发生多态行为**。

这部分已经在前文中有所介绍。

##### 多继承下的对象布局

当多继承中每个基类都有虚函数时，对象中会存在多个 vptr。

对于如下的实例：

```cpp
class A { public: virtual void f(){} };
class B { public: virtual void g(){} };
class C : public A, public B {int val; };
```

C 的对象结构大致如下：

```
vptr_A → A::vtable
vptr_B → B::vtable
A's members
B's members
C's members
```

##### 棱形继承

```cpp
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
```

在上面的代码中，`B1, B2`类简单继承了基类`B`，因此在派生类`D`中同时含有两个基类`B1, B2`中各自的基类`B`的信息，即间接继承了类`B`两次。这会导致一些歧义问题：

```cpp
// 错误，Base1与Base2中均含有变量base，因此歧义
std::cout << obj.base << std::endl;

// 正确
std::cout << obj.Base1::base << std::endl;
std::cout << obj.Base2::base << std::endl;
```

##### 虚继承和虚基类指针

为了解决棱形继承中的问题，

相对比较重要，挖坑，后续补充

#### 对象的生命周期如何管理？

构造时，从基类到派生类，顺序构造；析构时，从派生类到基类，逆序销毁；对象作用域结束时自动调用析构，这就是RAII的基础逻辑。

C++ 通过对象的生命周期模型，将“资源管理”绑定进语义层。这也是 C++ 能实现 RAII（Resource Acquisition Is Initialization）的根基。

在接下来第二部分内存管理中我们会展开较为深入的研究。

### 小结

| 概念      | 含义       | 内存表现             |
| ------- | -------- | ---------------- |
| 非静态成员变量 | 每个对象独立拥有 | 占用对象空间           |
| 静态成员变量  | 所有对象共享   | 存在全局区            |
| 成员函数    | 所有对象共用   | 存于代码段，不计入 sizeof |
| this 指针 | 指向当前对象   | 隐式参数             |
| vptr    | 指向虚函数表   | 每个含虚函数类的对象各有一份   |
| vtable  | 存放虚函数地址  | 每个类一张表           |

