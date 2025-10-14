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

要实现封装，最重要的是设定哪些公开，哪些不公开。在C++中，主要使用下面三个关键词来实现。

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
    类的同名函数**Adventure()**，当生成一个新的对象时自动执行，用于分配资源。

- 析构函数
    **~Adventure()**，正确释放所有动态分配的内存。
    构造函数和析构函数共同完成了内存管理，实现了**RAII**机制，后续会再次说明。

- get/change函数(const成员函数)
    用于从外部获取或修改某函数值。

上面的例子有几处危险的写法，这个我们后面再聊。

### 继承



### 多态

### 抽象