# Updates

续```part2-1.md```继续写

前面`part1`的一系列文章将C++面向对象的基本内容介绍完毕，接下来我们将探讨C++中有关内存管理的概念和方式方法。

接下来的内容主要包含，栈与堆的内存分配，`new/delete`与`malloc/free`，**RAII思想**，三/五法则和零法则。

## 引言

学习C++必须掌握内存管理，因为它是程序性能、稳定性与资源控制的核心。C++直接操作内存能实现高效资源利用，但也要求开发者承担内存安全责任。

## 栈与堆的内存分配

### 基本概念

要开始学习C++的内存管理，首先需要明确C++的内存概念。

C++的内存主要分为四个部分：代码区、全局/静态区、栈区、堆区。其中我们将要讨论的主要是栈区和堆区，也就是栈内存和堆内存。

| 内存区域          | 说明                                    |
| ------------- | ------------------------------------- |
| **代码区**       | 存放程序的机器指令（函数体、可执行代码）                  |
| **全局/静态区**    | 存放全局变量、静态变量、常量等                       |
| **栈区（Stack）** | 自动存储区。函数调用时系统自动分配，函数返回时自动释放           |
| **堆区（Heap）**  | 动态分配区。由程序员通过 `new` / `delete` 控制分配与释放 |

其中，栈内存由编译器自动管理，而堆内存可以由我们管理。

### 栈内存机制与特性

#### 栈内存的机制

栈内存是由系统自动管理的连续内存区域，其分配与释放与“数据结构”中的栈逻辑相同，均为严格的**后进先出**。

栈内存在程序运行中的主要用途为：
- 存储函数调用上下文(如`$sp`)
- 维护线程执行状态
- 通过CPU寄存器直接管理内存分配

#### 栈内存的特性

在机组的学习和写简单的汇编递归的经验之下，不难体会到栈内存首先具有的特性也是优势就是**分配/释放仅需移动`$sp`指针，因而是$O(1)$的**。

此外，栈内存的**生命周期与函数作用域严格绑定**，大小也是**固定步长**决定的。而且，栈内存的操作是由CPU寄存器直接操作，因此**访问速度极快**。

### 堆内存机制与特性

#### 堆内存的机制

堆内存是程序运行时可动态分配的内存区域，相比自动管理的栈内存，堆内存的设计是为了代码编写者更自由地对内存进行管理。

更自由的管理体现在内存块可以**以任意顺序分配和释放，支持动态调整大小，允许指针间接访问**。

不过自由之外还需要安全，因此要使用堆内存**必须在代码中显式请求分配/释放**。

#### 堆内存的特性

##### 自由的堆内存

相比于完全由系统自动管理的栈内存，堆内存是几乎完全由程序员来操作的内存。

当在代码中显式请求分配/释放时，堆内存就会开始/结束其生命周期。堆内存的生命周期从显式请求后一直**持续到显示释放或程序终止**。

而且，堆内存的容量**仅受系统可用内存限制**。

##### 自由的代价

堆内存固然有优势特性——自由，但是要实现这种自由，一定要付出 **“自由的代价”**。

其代价之一就是**内存碎片**。

内存碎片是指堆内存空间中存在许多小的、不连续的空闲内存块。这些内存块虽然总和很大，但是由于彼此并不连续，导致无法满足一次较大的内存分配请求。内存碎片可以主要分为外部碎片和内部碎片。其中从成因角度看，内部碎片是分配器出于对齐或管理方便的目的分配多于程序实际请求大小的内存而产生的，外部碎片则是分散在已分配内存块之间的空闲内存。我们通常关注的是**外部碎片**，下面的内存碎片也特指外部碎片。

我们来简单模拟一下内存碎片的产生过程。
```cpp
// 初始堆状态：[ 1024 Bytes Free ]

A = malloc(200);
B = malloc(100);
C = malloc(200);
D = malloc(100);
// [ A:200 ][ B:100 ][ C:200 ][ D:100 ][ Free:424 ]

free(A);
free(C);
// [ Free:200 ][ B:100 ][ Free:200 ][ D:100 ][ Free:424 ]

E = malloc(300);
// 寻找第一个大于 300 的内存碎片
// [ Free:200 ][ B:100 ][ Free:200 ][ D:100 ][ E:300 ][ Free:124 ]

F = malloc(250);
// 由于分隔开的内存碎片均小于 250
// 尽管总空闲 524 > 250，仍分配失败
```

如上就是一个内存碎片产生的可能机制。

内存碎片会导致内存的利用率降低，降低缓存的局部性。

##### 何以自由

从前面的机制介绍中思考，很自然就能体会到堆内存的特性与其**自由和自由的实现方式**强相关。

堆内存的核心机制包含了不同大小，不同生命周期的内存区域，要管理这样的区域就需要相应的分配算法，分配算法的取舍也能有效缓解诸如内存碎片的问题。

接下来我们尝试分析算法的设计要求。

对于分配算法的设计，主要关注点有三：
- 性能，即时间复杂度
- 碎片率
- 内存利用率

这三点构成了一个性能三角，不同的算法需要在这个三角中寻找一个平衡点。原本笔者期待在这里学习同时编写有关内存分配主流算法的思路，权衡，甚至实现，不过其内容涵盖完全可以作为单独一篇文章，因此挖坑，后续可能会出ex篇来介绍。

### 栈与堆的分配区别

来看下面一个实例：

```cpp
class Box {
public:
    Box(int v) : value(v) {
        std::cout << "Box constructed: " << value << '\n';
    }
    ~Box() {
        std::cout << "Box destructed: " << value << '\n';
    }
private:
    int value;
};

int main() {
    std::cout << "=== 栈上对象 ===\n";
    Box stackBox(1);

    std::cout << "=== 堆上对象 ===\n";
    Box* heapBox = new Box(2);

    std::cout << "=== 程序结束 ===\n";
    delete heapBox;
}
```

执行可以得到如下的输出：

```text
=== 栈上对象 ===
Box constructed: 1
=== 堆上对象 ===
Box constructed: 2
=== 程序结束 ===
Box destructed: 2
Box destructed: 1
```

在这个例子中我们可以明确感受到先前讲到的栈内存与堆内存的区别，并将其总述为下面的表格。

| 操作                | 内存区域 | 生命周期          | 释放方式   |
| ----------------- | ---- | ------------- | ------ |
| `Box stackBox(1)` | 栈    | 随作用域结束自动销毁    | 自动调用析构 |
| `new Box(2)`      | 堆    | 直到显式释放 | 必须手动释放 |

在使用堆内存的时候要小心隐式问题，主要是内存泄漏和内存多重释放。

## `new/delete`与`malloc/free`

首先，我们用一个表格来概括地介绍二者的区别，随后我们用几个代码来分别详细介绍。

|        | `malloc/free` | `new/delete` |
| --------- | ------------- | ------------ |
| 来源        | C 语言库函数       | C++ 运算符      |
| 是否类型安全    | 否             | 是            |
| 是否调用构造/析构 | 否             | 是            |
| 是否可重载     | 否             | 是            |
| 用途        | 原始内存操作        | 管理 C++ 对象    |
| 混用后果      | 未定义行为         | -            |

### `malloc/free`

`malloc/free`来源于C语言的库函数，作用为分配/释放原始内存。因而在使用过程中与C++面向对象契合度极低，不会自动调用构造/析构函数。

```cpp
Person *p = (Person *)malloc(sizeof(Person));
free(p);
// 仅分配对应大小内存，不会主动调用构造/析构函数
```

### `new/delete`

`new/delete`是C++的运算符，其功能就是分配并构造/析构并释放，因而是天然面向对象的。它们在一次执行中分别完成的任务为：调用`operator new`分配足够的内存，并调用构造函数初始化对象，最后返回对应指针；调用析构函数，随后调用`operator delete`释放内存。

我们用一个实例来验证上面的描述。

```cpp
class sample {
public:
    int data;
    char buffer[256];
    sample() {
        std::cout << "Constructor called." << std::endl;
    }

    ~sample() {
        std::cout << "Destructor called." << std::endl;
    }

    static void* operator new(size_t size) {
        std::cout << "Custom new operator called. Size: " << size << '\n';
        void* p = malloc(size);
        return p;
    }

    static void operator delete(void* p) {
        std::cout << "Custom delete operator called." << '\n';
        free(p);
    }
};

int main() {
    sample* obj = new sample;
    delete obj;
    return 0;
}
```

当我们运行上面的代码，可以得到如下的输出:

```text
Custom new operator called. Size: 260
Constructor called.
Destructor called.
Custom delete operator called.
```

这个输出与先前的描述是对应的。

### Placement new(定位new)

挖坑

## **RAII思想(资源获取即初始化)**

> **RAII(Resource Acquisition Is Initialization)** 是 C++ 的核心内存管理哲学。
> 它要求在对象构造时获取资源，在析构时释放资源，从而让资源的生命周期与对象的生命周期完全绑定。

简单来说，**RAII** 的主张就是“谁申请则谁销毁；构造时申请，析构时释放。”这种设计思想是为了解决传统资源管理中忘记关闭、释放等众多手动管理的复杂问题。

**RAII** 在内存管理、锁管理、数据库连接等多个场景下均有应用，同时还保证了异常安全、代码简洁。

接下来我们主要的讨论将限制在 **RAII** 思想在内存管理上的应用，进而**最核心、最常用**的也就是`std::unique_ptr, std::shared_ptr, std::weak_ptr`。

### `std::{unique, shared, weak}_ptr`概述

### `std::unique_ptr`

`std::unique_ptr` 是C++11引入的智能指针，是自动内存管理的核心工具，提供了异常安全的RAII语义，能够自动管理对象的生命周期，防止内存泄漏。

`std::unique_ptr`具有对动态分配内存对象的**独占所有权**——只有一个 `unique_ptr` 可以指向它。这一特性使得它**不支持拷贝，只支持转移所有权/移动语义**。当`unique_ptr`被销毁时会**自动释放资源**。而且在实现上面这些好处的同时，使用`unique_ptr`还是零开销的，提供便利而不牺牲性能。

#### 创建对象

我们下面的讨论均以`Sample`类为例，`Sample`类构造如下：

```cpp
class Sample {
public:
    Sample() { std::cout << "Sample Constructor" << std::endl; }
    ~Sample() { std::cout << "Sample Destructor" << std::endl; }
    void display() { std::cout << "Display Sample" << std::endl; }
};
```

创建一个 ``unique_ptr`` 管理 ``Sample`` 对象主要有四种方式：
1. 使用 `new`
    这种方法在出现异常时会造成内存泄漏。
    ```cpp
    std::unique_ptr<Sample> ptr(new Sample());
    ```
2. 默认构造函数创建一个空的 `unique_ptr`，使用 `reset` 方法分配新的对象
    ```cpp
    std::unique_ptr<Sample> ptr;
    ptr.reset(new Sample());
    ```
3. 直接传入裸指针
    这种方法可以直接接管已有的指针，但是需要注意不会重复释放。
    ```cpp
    Sample *rawptr = new Sample(); std::unique_ptr<Sample> ptr(rawptr);
    ```
4. 使用 `std::make_unique` 构造(推荐，C++14及以上)
    保证异常安全，避免内存泄漏，最推荐的方式。
    ```cpp
    std::unique_ptr<Sample> ptr = std::make_unique<Sample>();
    ```

#### 访问对象

访问对象的方式与常规的指针相似，都可以使用解引用操作符`*`和箭头操作符`->`。

具体演示在下面的完整实例中出现。

#### 所有权管理

前文提到，`std::unique_ptr`具有对动态分配内存对象的**独占所有权**。这种所有权只能转移或消亡。要对所有权进行操作主要有下面几种方式：
1. `std::move()`    ：转移指针所有权。
    ```cpp
    std::unique_ptr<Sample> ptr1 = make_unique<Sample>();
    std::unique_ptr<Sample> ptr2 = std::move(ptr1);
    // 之后 ptr1 变为 nullptr, ptr2 获得所有权
    ``
1. `release()`      ：释放`std::unique_ptr`的所有权到原生指针，此时`std::unique_ptr`不会自动释放内存，需要手动释放。
    ```cpp
    std::unique_ptr<Sample> ptr = make_unique<Sample>();
    int *rawptr = ptr.release();
    // 之后 ptr 变为 nullptr, rawptr 指向 ptr 原本对应的对象
    // 若要释放该内存必须使用 delete ptr;
    ```
1. `reset() / reset(make_unique<T>())`：对于前者，删除当前对象；对于后者，删除当前对象并将指针指向新对象。
    ```cpp
    std::unique_ptr<Sample> ptr = make_unique<Sample>();
    ptr.reset();                // 删除当前对象, ptr 变为 nullptr
    ptr.teset(new Sample());    // 删除当前对象, ptr 指向新对象
    ```
1. `std::swap()`：交换两指针的所有权。
    ```cpp
    std::unique_ptr<Sample> ptr1 = make_unique<Sample>();
    std::unique_ptr<Sample> ptr2 = make_unique<Sample>();
    std::swap(ptr1, ptr2);
    ```

相当一部分库函数需要传入裸指针，因而引入了`get()`成员函数，返回智能指针保存的裸指针。

#### 扩展

除了上面的基础功能以外，`std::unique_ptr`还能支持自定义删除器，支持数组，受篇幅限制此处不做展开。

### `std::shared_ptr`

`std::shared_ptr` 也是C++11引入的智能指针，用于管理动态分配的内存。

`std::shared_ptr` 通过引用计数机制实现多个指针共享同一个对象的所有权。当最后一个指向该对象的 `std::shared_ptr` 被销毁时，对象会被自动删除。

#### 创建对象

和 `std::unique_ptr` 类似，尽管存在构造函数可以用于常见对象，还是更推荐使用异常安全的 `std::make_shared<T>()` 创建此类对象。

创建一个 ``shared_ptr`` 管理 ``Sample`` 对象同样有四种方式：
1. 使用 `new`
    这种方法在出现异常时会造成内存泄漏。
    ```cpp
    std::shared_ptr<Sample> ptr(new Sample());
    ```
2. 默认构造函数创建一个空的 `shared_ptr`，使用 `reset` 方法分配新的对象
    ```cpp
    std::shared_ptr<Sample> ptr;
    ptr.reset(new Sample());
    ```
3. 直接传入裸指针
    这种方法可以直接接管已有的指针，但是需要注意不会重复释放。
    ```cpp
    Sample *rawptr = new Sample(); std::shared_ptr<Sample> ptr(rawptr);
    ```
4. 使用 `std::make_shared` 构造(推荐，C++14及以上)
    保证异常安全，避免内存泄漏，最推荐的方式。
    ```cpp
    std::shared_ptr<Sample> ptr = std::make_shared<Sample>();
    ```

#### 访问对象

和一般指针一样，也是基于`*`和`->`运算符进行操作。不作展开。

#### 管理对象

和`unique_ptr`不同，`shared_ptr`是一种**共享所有权**的智能指针。多个`shared_ptr`可以指向同一个对象，直到最后一个指向该对象的`shared_ptr`被销毁时才会释放该对象的资源。每个`shared_ptr`都有一个关联的计数器，通常称为引用计数。

此处需要注意，有关多个`shared_ptr`指向同一个对象的代码编写方式应当如下：
```cpp
std::shared_ptr<Sample> ptr1 = std::make_shared<Sample>(42);
std::shared_ptr<Sample> ptr2 = ptr1;
std::shared_ptr<Sample> ptr3(ptr1);
```

而不是：
```cpp
Sample *p = new Sample(42);
std::shared_ptr<Sample> ptr1(p);
std::shared_ptr<Sample> ptr2(p);
```

下面的错误样例实际上构造了两组`shared_ptr`管理同一个裸指针对应的内存，这样操作会导致一系列问题。要使用多个`shared_ptr`指向同一个对象，只能采用上面样例的方式，也就是拷贝构造和拷贝赋值。

除此以外，`shared_ptr`还可以执行移动构造和移动复制，例如下面的实例：
```cpp
std::shared_ptr<Sample> ptr1 = std::make_shared<Sample>(42);
std::shared_ptr<Sample> ptr4 = ptr1;            // Shared ownership again
std::shared_ptr<Sample> p = std::move(ptr1);    // Transfer ownership to p
std::cout << "Reference Count after move(Count from p): " << p.use_count() << '\n';
std::cout << "Reference Count after move(Count from ptr4): " << ptr4.use_count() << '\n';
std::cout << "Is ptr1 null after move? " << (ptr1 == nullptr ? "Yes" : "No") << '\n';
```

运行上面的代码得到输出如下：
```text
Reference Count after move(Count from p): 2
Reference Count after move(Count from ptr4): 2
Is ptr1 null after move? Yes
```

#### 常用操作

有关`shared_ptr`的使用，主要有以下操作：
1. `use_count()`：返回当前`shared_ptr`的强引用计数。
    ```cpp
    std::shared_ptr<Sample> ptr1 = std::make_shared<Sample>();
    std::shared_ptr<Sample> ptr2 = ptr1;
    std::cout << ptr1.use_count(); // 输出2，因为ptr1和ptr2共享所有权
    ```
2. `unique()`：检查当前`shared_ptr`是否是唯一拥有该对象的智能指针（即`use_count() == 1`）。
    ```cpp
    std::shared_ptr<Sample> ptr1 = std::make_shared<Sample>();
    if (ptr1.unique()) {
        // 此时只有ptr1拥有对象
    }
    std::shared_ptr<Sample> ptr2 = ptr1;
    if (!ptr1.unique()) {
        // 现在有多个shared_ptr拥有该对象
    }
    ```
3. `reset()`：放弃当前`shared_ptr`对对象的所有权，减少引用计数。如果引用计数变为0，则删除对象。
    ```cpp
    std::shared_ptr<Sample> ptr1 = std::make_shared<Sample>();
    std::shared_ptr<Sample> ptr2 = ptr1;
    ptr1.reset(); // ptr1放弃所有权，引用计数减1，ptr1变为nullptr
    // 对象仍然存在，因为ptr2还在引用它
    ```
4. `get()`：返回内部保存的裸指针，用于与需要裸指针的API交互。
    ```cpp
    std::shared_ptr<Sample> ptr = std::make_shared<Sample>();
    Sample* rawPtr = ptr.get();
    // 可以使用rawPtr，但不能手动delete它
    ```
5. `std::swap()`：交换两个`shared_ptr`所管理的对象。
    ```cpp
    std::shared_ptr<Sample> ptr1 = std::make_shared<Sample>();
    std::shared_ptr<Sample> ptr2 = std::make_shared<Sample>();
    std::swap(ptr1, ptr2); // 交换ptr1和ptr2管理的对象
    ```

#### 循环引用问题

前面我们提到，引用计数增加需要通过拷贝构造/拷贝复制来进行。这时候很显然可以“灵光一现”，有没有可能有两个或多个`shared_ptr`相互引用，最终导致无法释放。

实际上这个问题并非空穴来风。`shared_ptr`的使用中存在循环引用问题，最终导致多个`shared_ptr`无法清空引用计数，进而无法自动释放空间，产生内存泄漏。

下面的一个实例尝试复现了一个简单的循环引用问题。
```cpp
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
```

在上面的类定义中，执行下面的代码：
```cpp
std::shared_ptr<SampleA> a = std::make_shared<SampleA>();
std::shared_ptr<SampleB> b = std::make_shared<SampleB>();
a->b_ptr = b;
b->a_ptr = a;
std::cout << "Reference Count of SampleA: " << a.use_count() << '\n';
std::cout << "Reference Count of SampleB: " << b.use_count() << '\n';
```

得到输出如下：
```text
SampleA Constructor
SampleB Constructor
Reference Count of SampleA: 2
Reference Count of SampleB: 2
```

可以看出，构造`SampleA, SampleB`类的`shared_ptr`之后，二者形成循环引用，表现为引用计数均为2。在离开作用域后没有输出二者析构函数被调用的输出，即两个对象都没有被销毁，产生了内存泄漏。

### `std::weak_ptr`

`std::weak_ptr` 是 C++11 引入的智能指针，用于**弱引用（non-owning reference）**对象。它与 `std::shared_ptr` 搭配使用，用来**观察**由 `shared_ptr` 管理的对象，却**不参与对象生命周期管理**，也不会增加引用计数。

它的主要目的就是，**避免 shared_ptr 循环引用导致的内存泄漏**。

因为它不增加引用计数，因此不会阻止对象的析构。当对象已经被销毁时，`weak_ptr` 也能安全检测出这一点。

#### 创建对象

首先需要注意的是，`weak_ptr` **不能直接管理裸指针**，因此其创造对象的方式与先前的`unique_ptr, shared_ptr`均有所不同。

创建方式通常有三种：
1. 从已有 `shared_ptr` 构造（最常用）
    ```cpp
    std::shared_ptr<Sample> sp = std::make_shared<Sample>();
    std::weak_ptr<Sample> wp(sp);
    ```
2. 通过赋值操作生成弱引用
    ```cpp
    std::weak_ptr<Sample> wp;
    wp = sp;
    ```
3. 通过 `std::weak_ptr` 拷贝
    ```cpp
    std::weak_ptr<Sample> wp1(sp);
    std::weak_ptr<Sample> wp2(wp1);
    ```

此外注意：weak_ptr **不会改变引用计数**，即不会影响 shared_ptr 共享对象的生命周期。

#### 访问对象

`weak_ptr` 本身不具备对对象的所有权，因而不能直接使用 `->` 或 `*` 访问。必须先使用`lock()`成员函数将其“升级”为 `shared_ptr` 才能使用。

成员函数`lock()`会先检查`weak_ptr`指向的对象是否存在，如存在则返回指向该对象的`shared_ptr`，如不存在则返回值为`nullptr`的`shared_ptr`。如下面的实例：
```cpp
if (auto sp = wp.lock()) {   // lock() 成功：对象还存在
    // do something
} else {
    std::cout << "Object no longer exists\n";
}
```

具体地说，`weak_ptr.lock()` 的行为方式如下：

| 情况       | lock() 返回               |
| -------- | ----------------------- |
| 管理对象仍然活着 | 一个新的 shared_ptr（引用计数+1） |
| 对象已被销毁   | 空 shared_ptr            |

#### 管理对象

与 `shared_ptr` 不同；`weak_ptr` **不增加引用计数， 不会阻止对象析构，不能直接访问对象**。

因此，它是纯观察者，适用于：监控对象是否仍然存在，临时访问对象（通过 lock 升级），避免 shared_ptr 循环引用。

示例：

```cpp
std::shared_ptr<Sample> sp = std::make_shared<Sample>();
std::weak_ptr<Sample> wp = sp;

sp.reset();   // 销毁对象

if (wp.expired()) {
    std::cout << "Object already destroyed\n";
}
```

#### 常用操作

`weak_ptr`的使用，主要有以下操作：
1. `expired()`：检查`weak_ptr`所观察的对象是否已被释放。
    ```cpp
    std::weak_ptr<Sample> weak;
    {
        std::shared_ptr<Sample> shared = std::make_shared<Sample>();
        weak = shared;
        std::cout << weak.expired(); // 输出0（false），对象还存在
    }
    // shared离开作用域，对象被释放
    std::cout << weak.expired(); // 输出1（true），对象已被释放
    ```
2. `lock()`：检查所观察对象是否仍存在。如存在则返回指向该对象的`shared_ptr`，如不存在则返回值为`nullptr`的`shared_ptr`。
    ```cpp
    std::weak_ptr<Sample> weak;
    {
        std::shared_ptr<Sample> shared = std::make_shared<Sample>();
        weak = shared;
        if (auto ptr = weak.lock())
        {
            // do something
        }
        else std::cout << "Object no longer exists";
    }
    ```
3. `reset()`：释放`weak_ptr`对所观察对象的引用，将其置为空状态。
    ```cpp
    std::shared_ptr<Sample> shared = std::make_shared<Sample>();
    std::weak_ptr<Sample> weak = shared;
    
    std::cout << weak.expired(); // 输出0（false）
    weak.reset();                // 释放观察引用
    std::cout << weak.expired(); // 输出1（true），weak不再观察任何对象
    ```
4. `use_count()`：返回当前观察对象的`shared_ptr`的强引用计数。
    ```cpp
    std::shared_ptr<Sample> shared1 = std::make_shared<Sample>();
    std::weak_ptr<Sample> weak = shared1;
    std::cout << weak.use_count(); // 输出1
    
    std::shared_ptr<Sample> shared2 = shared1;
    std::cout << weak.use_count(); // 输出2
    ```
5. `std::swap()`：交换两个`weak_ptr`所观察的对象。
    ```cpp
    std::shared_ptr<Sample> shared1 = std::make_shared<Sample>();
    std::shared_ptr<Sample> shared2 = std::make_shared<Sample>();
    
    std::weak_ptr<Sample> weak1 = shared1;
    std::weak_ptr<Sample> weak2 = shared2;
    
    std::swap(weak1, weak2); // 交换weak1和weak2观察的对象
    // 现在weak1观察shared2，weak2观察shared1
    ```
6. `owner_before()`：提供基于所有权的排序，用于在关联容器中使用`weak_ptr`作为键。
    ```cpp
    std::shared_ptr<Sample> shared1 = std::make_shared<Sample>();
    std::shared_ptr<Sample> shared2 = std::make_shared<Sample>();
    
    std::weak_ptr<Sample> weak1 = shared1;
    std::weak_ptr<Sample> weak2 = shared2;
    
    // 基于所有权的比较
    if (weak1.owner_before(weak2)) {
        // weak1在所有权顺序上位于weak2之前
    }
    ```
#### 解决循环引用



## 三/五法则和零法则

