# Updates

- 2025/10/16: 完成面向对象部分第一版。

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

如下是一个简单的类实现：

```cpp
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
            delete this;
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

需要注意的是，上面的例子有一处危险的写法```delete this;```，具体原因和风险我们后面再聊。

### 继承

继承是指，依据另一个类来定义一个类。通过继承我们可以更简单高效的复用代码。

#### 单继承

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

#### 多继承

To be continued...

#### 继承中的注意事项

当创建派生类对象时，首先调用基类构造函数，然后调用派生类构造函数。析构则反序。这一特点可以在```sample_1_virtualWithOverride.cpp```的输出结果中看出。

### 多态

多态的核心在于，让同一接口对于不同对象表现出不同的行为。

在C++中，多态分为两种实现方式：静态多态，动态多态。

| 类型       | 发生时间 | 实现方式           | 例子                                   |
| -------- | ---- | -------------- | ------------------------------------ |
| **静态多态** | 编译期  | 模板、函数重载        | `std::sort()` 可对任何类型排序               |
| **动态多态** | 运行期  | 虚函数（`virtual`） | `Animal* p = new Dog(); p->speak();` |

在OOP中，我们主要讨论动态多态的实现方式。

#### 虚函数

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

**override**关键字是一个用于**降低风险**的关键字，只要包含**override**关键字，则表明该函数是基类中已有声明的虚函数的重写，可以规避静态重载的可能。例如如下的代码：

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
};

class Dog : public Animal {
public:
    Dog(const char *name, int age) : Animal(name, age) {
        std::cout << "Dog " << name << " created.\n";
    }

    ~Dog() override {
        std::cout << "Dog destroyed.\n";
    }

    void makeSound() {
        std::cout << "Woof! Woof!\n";
    }
};

```
在上面的实例中如果我们用如下的方式操作。

```cpp
Animal *d = new Dog("Buddy", 3);
d->makeSound();
delete d;
```

则输出如下

```
Animal Buddy created.
Dog Buddy created.
Animal sound!
Dog destroyed.
Animal Buddy destroyed.
```

可以看到输出中出现了```Animal sound!```，说明我们在```d->makeSound();```这一行中调用的函数并不是在```class Dog```中**尝试重写**的那个。详细分析代码可以发现，由于两个函数声明上的区别（```void Animal::makeSound const()```与```void Dog::makeSound()```），编译器将两个函数分别认为两个独立函数去思考，而并没有认为是基类中虚函数的重写。

因此就可以看出**override**在这个过程中的作用：明确告知编译器，此函数是基类虚函数的重写。在编译时，编译器会主动寻找基类中的虚函数，并搭建虚函数表，如果类型有出入则提示编译错误。从而，可以规避不影响代码跑通的隐形错误。

#### 虚函数表

当类中含有虚函数时，编译器会生成虚函数表(vtable)和虚表指针(vptr)。

##### 虚表指针的存在

```cpp
class A {
public:
    void function() {
        std::cout << "This is A::function()" << '\n';
    }
};

class B {
public:
    virtual void function() {
        std::cout << "This is B::function()" << '\n';
    }
};
```

```cpp
A a;
B b;
std::cout << "Size of class A: " << sizeof(a) << '\n';
std::cout << "Size of class B: " << sizeof(b) << '\n';
```

针对如上的类，我们运行下方的代码，得到类A与B的大小分别为**1**和**8**。C++标准要求所有对象都需要有独特的地址，因此对空类A分配了大小为**1**的空间；而对于类B，由于其中含有虚函数，所以在生成该类时同时生成了虚表指针，该指针占据空间为**8**，因而我们得知虚表指针是存在的。

##### 虚表指针的生成

虚表指针既然存在，还占据空间，那么一定会生成，也就是初始化。经查阅资料可以得知，含有虚函数的类在生成时会经历如下的过程：

- 对象内存分配
    为该类的所有对象（所有数据成员和虚表指针）分配内存。

- 虚表指针初始化
    进入构造函数代码的执行阶段之前，编译器生成的代码会先将虚表指针指向该类的虚表（一个类具有一个固定虚函数表）。

- 构造函数的执行
    构造函数如有初始化列表，则先执行初始化列表以初始化类的数据成员，随后执行构造函数内的代码。

根据上面的流程可以知道，虚表指针在构造函数中被初始化，进而，一个类的构造函数是不能成为虚函数的。

##### 虚函数表

类与虚函数表式、是一一对应关系，一个类具有一个固定虚函数表。

在单继承中，派生类仅会初始化一个虚表指针，指向其基类的虚函数表；在多继承中，派生类会根据基类的数量生成对应数量的虚表指针。

来看下面的例子：

```cpp
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
```

在这个实例中，执行
```cpp
std::cout << "Size of class C: " << sizeof(c) << '\n';
```
得到```Size of class C: 16```。这说明在继承了基类A和基类B的派生类C中含有两个虚表指针，分别指向基类A与基类B的虚函数表。

总结起来，每个有虚函数的基类拥有独立vptr，因此多继承下每个子对象各维护一份vptr，正如上文中派生类C的对象实际包含两个虚表指针。

#### 虚函数调用在构造/析构函数中的限制

构造/析构函数中调用虚函数，不会发生动态绑定，而是调用当前类版本。（当前类即正在执行构造/析构函数的类）

为了解释上面这句话，我们需要首先再此介绍一遍“动态绑定”的含义。动态绑定是C++实现多态的核心机制，其使得代码在运行时根据对象的实际类型来决定调用哪个虚函数版本。

对于基类```Base```与派生类```Derived```的完整实例化对象的过程中，会依次执行```Base构造 → Derived构造 → 对象完全建立 → Derived析构 → Base析构```，因此在构造/析构函数中，我们很难确定当前数据是什么类型（在```Base```构造函数中，由于```Derived```尚未构造，对象就不是一个完整的```Derived```对象；在```Base```析构函数中，由于```Derived```已被销毁，对象就已不再是一个```Derived```对象）。

如果在这些时刻允许动态绑定调用```Derived```版本的函数，可能会访问到未初始化或已销毁的成员，导致未定义行为。

因此总的来说，最好不要在构造/析构函数中调用虚函数，因为其影响可能是不可控的。

#### 总结表格

| 概念         | 功能                        |
| ---------- | ------------------------- |
| `virtual`  | 告诉编译器“这是可被重写的函数”          |
| `override` | 告诉编译器“我要重写基类的 virtual 函数” |
| `vptr`     | 每个对象内一个隐藏指针，指向对应类的 vtable |
| `vtable`   | 存放虚函数地址的表，运行时通过它实现动态绑定    |

### 抽象

抽象是一种思想：只向外界提供最核心的接口，不需要完成具体的实现。

笔者认为，抽象更像是在提取特征。这些特征本身并不能构成已有的物体，但是可以用这些特征来构成物体，并在实现物体类时详细描述这些特征。

#### C++中的抽象

在C++中，我们可以使用抽象类来实现。

抽象类是指该类中至少有一个函数被声明为纯虚函数。

纯虚函数的声明方式如下：

```cpp
class areaCalculator {
public:
    // 纯虚函数声明
    virtual double getArea() const = 0;
    // 虚析构函数
    virtual ~areaCalculator() = default;
};

class shape {
protected:
    double width;
    double height;
public:
    shape(double w, double h) : width(w), height(h) {}
    virtual ~shape() = default;
};

class rectangle : public shape, public areaCalculator {
public:
    rectangle(double w, double h) : shape(w, h) {}
    double getArea() const override {
        return width * height;
    }
    ~rectangle() = default;
};
```

在该实例中，我们定义了一个正方形类```rectangle```，它具有两个特征，一个是形状类```shape```，一个是面积可计算```areaCalculator```。其中```areaCalculator```中含有纯虚函数```double getArea() const```，因而是一个抽象类。

我们执行下面的代码段：

```cpp
areaCalculator* ac = nullptr;
ac = new areaCalculator();
```

会发现在编译第二行时出现报错，这是因为抽象类是不能实例化对象的。

在实际工程中，我们可以提前实现若干结构，比如对于一个形状，其拥有自己的独特参数，但是可计算面积(calculable)，可绘制(drawable)，可以拥有很多性质。我们可以提前实现抽象类```Calculable```和```Drawable```，实现更高效的代码复用，以及更方便的代码维护。

#### 抽象与封装的区别

其实仅仅看概念，笔者的第一个疑问就是，抽象和封装有什么区别。在实际上手及查阅资料后，笔者试对其概念进行区别。

- 抽象强调的是提取特征，构建对象
    抽象的根本目的是将复杂的现实原型转变为代码，而且是有逻辑的代码。因此需要经过观察和思考后，定义一个个特征模型类。不同的事物类具有不同的特征模型类集，不同的特征模型类也可以分别对应多个事物类，从而构成了事物类集和特征模型类集的二分图。
    这种思想使得，我们可以提取特征并预留结构，当需要补充、丰富已有代码时可以直接调整特征模型类，而**不需逐个**对于具有本特征的事物类进行修改调整，方便代码扩展和维护。
- 封装强调的是权限管理，保护数据
    封装是类的核心思想，其实现了数据和函数的打包。这一思想的关键在于权限管理：通过关键字```private, protected, public```对外部用户隐藏内部细节，从而保证数据安全。
