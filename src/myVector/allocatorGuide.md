# C++ 内存管理：从 Primitive 到 Allocator

在实现标准库容器（如 `vector`）时，深入理解 C++ 的内存管理机制是必不可少的。本文将详细对比底层的 **C++ 原语 (Primitives)** 与现代 STL 的 **分配器 (Allocator)** 抽象。

## 1. 原始内存管理 (C++ Primitives)

`new` 和 `delete` 在执行过程中实际上包含两个步骤：内存分配和对象构造。接下来，我们将两个步骤分别拆解，以更好的解释 C++ 原始内存管理方式。

### 1.1 `malloc` / `free`

有关 `malloc` / `free` 的函数原型定义如下：
```c
void* malloc(size_t size);
void free(void* ptr);
```

具体使用过程如下面的一个实例：
```cpp
// 1. 分配内存 (不会调用构造函数)
int* p = (int*)malloc(10 * sizeof(int));
if (p == nullptr) { /* 处理分配失败 */ }
// 2. 释放内存 (不会调用析构函数)
free(p);
```

有关这组内存分配方式的特点，首先，它们是**纯粹的内存操作**，仅仅是向操作系统申请和归还字节，而完全不理解有关“对象”或“类型”的概念。因此，如果要作用于 C++ class 对象，由于其不会调用构造/析构函数，会导致未经初始化和资源泄漏的错误。

### 1.2 `operator new` / `operator delete`

这是 C++ 提供的运算符（也可认为是全局函数），是 `malloc` 的 C++ 封装与替代品。注意它与关键字 `new` 不同。

`operator new` / `operator delete`的原型定义如下：
```cpp
void* operator new(size_t size);
void operator delete(void* ptr) noexcept;
```

如下是一个实例：
```cpp
// 1. 仅分配内存，不构造对象
// 等同于 malloc，但失败会抛出 std::bad_alloc 异常
int* raw_memory = static_cast<int*>(::operator new(10 * sizeof(int)));
// 此时 raw_memory 指向的内存是未初始化的

// 2. 使用完毕后释放内存
::operator delete(raw_memory);
```

这种分配方式可以得到**异常安全的空白内存**（不经过初始化），且可以由用户重载类内的 `operator new` 来实现针对特定类的内存池优化。

### 1.3 `new` / `delete`

这是我们最常用的 C++ 关键字。

基本使用方式：
```cpp
T* ptr = new T(args...);  // 分配 + 构造
delete ptr;               // 析构 + 释放
```

有关 `new` / `delete` 最需要注意的就是其操作不再仅仅是原始内存的分配，而是：
1. `new` 先分配内存，然后在内存上执行构造函数。
2. `delete` 先执行析构函数，然后释放对应内存。

### 1.4 Placement New

这是 `new` 表达式的一个特殊变体。它**不分配内存**，而是**在已有的内存地址上**构造对象。它在 C++ 中实现了内存分配与对象构造的解耦，也是内存池的基本原理。

基本使用方式及使用示例：
```cpp
// ----- 基本使用方式 -----
#include <new> // 必须引入
new (address) Type(arguments...);

// ----- 使用示例 -----
class MyClass {
public:
    MyClass(int x) { cout << "Construct: " << x << endl; }
    ~MyClass() { cout << "Destruct" << endl; }
};

// 1. 准备内存 (Step 1: Allocation)
void* mem = ::operator new(sizeof(MyClass));

// 2. 在 ptr 指向的内存上构造对象 (Step 2: Construction)
// 这就是 vector::push_back 内部做的事情
MyClass* ptr = new (mem) MyClass(42); 

// 3. 显式调用析构函数 (Step 3: Destruction)
// 必须手动调用！因为 delete ptr 会尝试释放内存，而我们只想析构对象
ptr->~MyClass();

// 4. 释放原始内存 (Step 4: Deallocation)
::operator delete(mem);
```

placement new 的优秀之处可以使用一次分配内存而多次构造对象，能够用以实现优秀的内存复用机制，比如：
1. 在标准容器中可以实现对象消除后，内存并未归还系统，下一次构造操作是在已有的内存上执行 placement new。
2. 可以实现内存池：在预先分配好的一大块内存上按需构造对象，避免频繁的系统调用开销。

## 2. 分配器抽象 (Allocator Abstraction)

直接使用 `operator new` 的问题在于它将内存来源“硬编码”为堆内存。为了支持自定义内存模型（如栈分配、共享内存、显存分配），STL 引入了 `Allocator`。

但 Allocator 的接口随着 C++ 标准演进（C++98 -> C++11 -> C++17 -> C++20）发生了很大变化。为了屏蔽这些差异，标准库提供了 **`std::allocator_traits`**。

### 2.1 `std::allocator_traits`
这是访问 Allocator 的**统一标准代理（Proxy）**。你应当总是通过 `traits` 来调用 Allocator，而不是直接调用 Allocator 的成员函数。

**基本语法**:
```cpp
#include <memory>
using Traits = std::allocator_traits<Alloc>;

// 核心接口
Traits::allocate(alloc, n);                 // 分配
Traits::deallocate(alloc, p, n);            // 释放
Traits::construct(alloc, p, args...);       // 构造
Traits::destroy(alloc, p);                  // 析构
```

**使用示例 (实现一个简易 Vector 的核心部分)**:
```cpp
template <typename T, typename Alloc = std::allocator<T>>
class MyVector {
    Alloc alloc; // 持有一个分配器实例
    T* data;

public:
    void push_back(const T& val) {
        // 1. 分配 (如果需要)
        // data = std::allocator_traits<Alloc>::allocate(alloc, 1);
        
        // 2. 构造
        // 等价于: new (data) T(val)
        // 但 Allocator 可能对指针做了特殊处理（Fancy Pointers）
        std::allocator_traits<Alloc>::construct(alloc, data, val);
    }

    ~MyVector() {
        // 3. 析构
        // 等价于: data->~T()
        std::allocator_traits<Alloc>::destroy(alloc, data);

        // 4. 释放
        // 等价于: operator delete(data)
        std::allocator_traits<Alloc>::deallocate(alloc, data, 1);
    }
};
```

**特点与使用场景**:
*   **通用性接口**: 它会自动检测 Allocator 是否提供了特定的成员函数（如 `construct`）。如果有，就调用它；如果没有，就回退到默认实现（即 placement new）。这意味着你写的自定义 Allocator 可以非常简单，只实现 `allocate` 和 `deallocate` 即可，其他都由 Traits 自动补全。
*   **状态处理**: 处理 `select_on_container_copy_construction` 等复杂的 Allocator 传播逻辑（Propagate on Copy/Move/Swap），这是手写 `operator new` 无法做到的。
*   **工业级标准**: 所有 STL 容器（`std::vector`, `std::map` 等）都强制通过 `allocator_traits` 交互。

## 总结：该选哪种？

| 维度 | C++ Primitives (`operator new` + `placement new`) | Allocator Traits (`std::allocator_traits`) |
| :--- | :--- | :--- |
| **灵活性** | 低。只能在 Heap 上分配。 | 高。可切换到 Stack, Pool, Shared Memory。 |
| **代码复杂度** | 中等。手动管理生命周期。 | 较高。需要引入模板参数和 traits。 |
| **封装性** | 差。直接暴露底层细节。 | 优。通过 Policy 模式解耦。 |
| **适用场景** | 教学、简单的资源管理类、不需要自定义分配器的场景。 | **编写通用容器**、需要高性能内存池、嵌入式系统受限内存。 |

对于我们的 `myVector` 项目，从 Phase 2 迁移到 Phase 3，正是从“教学玩具”向“库级别组件”跃升的过程。
