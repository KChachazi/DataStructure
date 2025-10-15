# 面向对象与内存管理

## 面向对象概述

面向对象编程，简称 OOP，是一种编程范式。其核心思想是：将程序组织为一系列对象的集合，每个对象都是类的实例，对象之间通过消息传递进行交互。

相比于面向过程编程，其在运行阶段进行决策更为容易。如考虑为数组分配内存，可以动态管理。这一特点并非OOP独有，但是在C++中编写的难度要远低于C。

面向对象有如下的优点：

1. 便于理解与维护。

1. 代码复用性高。

1. 可扩展性强。

1. 高内聚，低耦合。

## C++面向对象的四块基石

### 封装

封装是指，将对象的属性（数据）和行为（方法）包装在一起，并**隐藏**内部的实现细节，仅对外**公开**一个可控的访问接口。其核心在于，你无需关注其内部实现，而只需知道其如何使用。这样的好处在于，一方面可以确保内部数据不会被外部侵染，从而保障数据安全，另一方面可以简化使用者的使用。

要实现封装，最重要的是设定哪些公开，哪些不公开。在C++中，主要使用下面三个关键字来实现。

- ```private```：私有成员，仅在类内部访问。
- ```public```：公有成员，可以在任何地方方位。
- ```protected```：保护成员，可以在继承中使用。

如下是一个简单的类实现（```sample_1_Adventurer.cpp```）：

```cpp
#include <iostream>
#include <cstdio>

class Adventurer {
private:
    char *name_;
    int *level_, *health_, *attack_;

public:
    // 构造函数
    Adventurer(const char *name, int level, int health, int attack)
        : level_(new int(level)), health_(new int(health)), attack_(new int(attack)) {
        name_ = new char[strlen(name) + 1];
        std::strcpy(name_, name);
        std::cout << "Adventurer " << name_ << " created.\n";
    }

    // 析构函数
    ~Adventurer() {
        delete[] name_;
        delete level_;
        delete health_;
        delete attack_;
        std::cout << "Adventurer " << name_ << " destroyed.\n";
    }

    // 返回生命值
    int getHealth() const {
        return *health_;
        // const成员函数保证不修改对象状态
    }

    // 返回攻击力
    int getAttack() const {
        return *attack_;
    }

    // 返回等级
    int getLevel() const {
        return *level_;
    }

    // 返回名字
    const char* getName() const {
        return name_;
    }

    // 生命值变化
    void changeHealth(int delta) {
        *health_ += delta;
        if (*health_ < 0) *health_ = 0;
        std::cout << name_ << "'s health changed to " << *health_ << ".\n";
        if (*health_ == 0) {
            std::cout << name_ << " has been killed!\n";
            delete this; // 自我销毁
        }
    }

    // 攻击力变化
    void changeAttack(int delta) {
        *attack_ += delta;
        if (*attack_ < 0) *attack_ = 0;
        std::cout << name_ << "'s attack changed to " << *attack_ << ".\n";
    }

    // 显示状态
    void displayStatus() const {
        std::cout << "Adventurer: " << name_ << "\n"
                  << " Level: " << *level_ << "\n"
                  << " Health: " << *health_ << "\n"
                  << " Attack: " << *attack_ << "\n";
    }
};
```

在这个例子中，包含了构造函数、析构函数、getData、changeData、displayStatus，可以用来简单介绍类的基本构成：

- 成员变量
    一般为**private**，不允许外部访问。

- 构造函数
    类的同名函数```Adventure()```，当生成一个新的对象时自动执行，用于分配资源。

- 析构函数
    ```~Adventure()```，正确释放所有动态分配的内存。
    构造函数和析构函数共同完成了内存管理，实现了**RAII**机制，后续会再次说明。

- ```get/change```函数(const成员函数)
    用于从外部获取或修改某函数值。

上面的例子有几处危险的写法，这个我们后面再聊。

### 继承

继承是指，依据另一个类来定义一个类。通过继承我们可以更简单高效的复用代码。

如下是一个典型的继承例子：

```cpp
class Animal {
private:
    char *name_;
    int age_;
public:
    Animal(const char *name, int age) : age_(age) {
        name_ = new char[strlen(name) + 1];
        std::strcpy(name_, name);
        std::cout << "Animal " << name_ << " created.\n";
    }

    ~Animal() {
        std::cout << "Animal " << name_ << " destroyed.\n";
        delete[] name_;
    }

    const char* getname() const {
        return name_;
    }
    void eat() const {
        std::cout << name_ << " is eating.\n";
    }
    void sleep() const {
        std::cout << name_ << " is sleeping.\n";
    }
};

class Dog : public Animal {
public:

    Dog(const char *name, int age) : Animal(name, age) {
        std::cout << "Dog " << name << " created.\n";
    }
    
    void bark() const {
        std::cout << "Woof! Woof!\n";
    }
};
```

在这个例子中，**Dog**作为**Animal**的**派生类**，**Animal**作为**Dog**的**基类**。

作为一个派生类，**Dog**在其中拥有的基类的变量有：

- $\text{private}$(拥有但不可访问):
    ```char *name_;```
    ```int age_;```

- $\text{public}$(可以在类外自由访问):
    ```Animal();```   //构造函数
    ```~Animal();```  //析构函数
    ```const char* getName();```
    ```void eat();```
    ```void sleep();```

- $\text{protected}$(可以在派生类中访问，但类外不可访问，本例中不存在)

### 多态

多态的核心在于，让同一接口对于不同对象表现出不同的行为。

在C++中，多态分为两种实现方式：静态多态，动态多态。

| 类型       | 发生时间 | 实现方式           | 例子                                   |
| -------- | ---- | -------------- | ------------------------------------ |
| **静态多态** | 编译期  | 模板、函数重载        | `std::sort()` 可对任何类型排序               |
| **动态多态** | 运行期  | 虚函数（`virtual`） | `Animal* p = new Dog(); p->speak();` |

在OOP中，我们主要讨论动态多态的实现方式。

#### 虚函数与虚函数表

虚函数的关键字为**virtual**，声明一个虚函数的方式如下：

```cpp
class Animal {
    std::string name_;
    int age_;
public:
    Animal(const char *name, int age) : name_(name), age_(age) {
        std::cout << "Animal " << name_ << " created.\n";
    }

    virtual ~Animal() {
        std::cout << "Animal " << name_ << " destroyed.\n";
    }

    virtual void makeSound() const {
        std::cout << "Animal sound!\n";
    }

    const std::string getName() const {
        return name_;
    }
};

class Dog : public Animal {
public:
    Dog(const char *name, int age) : Animal(name, age) {
        std::cout << "Dog " << name << " created.\n";
    }

    ~Dog() override {
        std::cout << "Dog destroyed.\n";
    }

    void makeSound() const override {
        std::cout << "Woof! Woof!\n";
    }
};
```

在如上的案例中，我们在基类中声明了虚函数```virtual ~Animal()```, ```virtual void makeSound()```，表明在该基类的派生类中可以重写这些函数。

重写函数时，我们通常会使用**override**关键字来表明该函数是基类虚函数的重写。

#### **override**关键字

**override**关键字是一个用于**降低风险**的关键字，只要包含override关键字，则表明该函数是基类中已有声明的虚函数的重写，可以规避静态重载的可能。例如如下的代码：

```cpp
class Mom {
protected:
    string name_;
public:
    ~Mom() { }
    const string getName() const {
        return name_;
    }
}

class Child : public Mom {
    string getName() {
        return name_;
    }
}

```

#### 总结表格

| 概念         | 功能                        |
| ---------- | ------------------------- |
| `virtual`  | 告诉编译器“这是可被重写的函数”          |
| `override` | 告诉编译器“我要重写基类的 virtual 函数” |
| `vptr`     | 每个对象内一个隐藏指针，指向对应类的 vtable |
| `vtable`   | 存放虚函数地址的表，运行时通过它实现动态绑定    |


### 抽象