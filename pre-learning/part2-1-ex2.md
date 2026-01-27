# placement new

下面我来给你讲一次 **“真正深入、工程视角、语言机制视角三合一版本的 placement new”**。
这将比网上绝大多数文章都更系统，你可以直接放入你的文档作为“权威讲解”。

---

# 🌟 什么是 placement new？

> **placement new（定位 new）是一种特殊形式的 new，用来在 *指定的内存地址* 上构造对象，而不是分配新内存。**

语法：

```cpp
void* operator new (std::size_t, void* place) noexcept;
```

调用形式：

```cpp
T* p = new (address) T(args...);
```

也就是说：

| 普通 new      | placement new               |
| ----------- | --------------------------- |
| 分配内存 + 构造对象 | **不分配内存，只调用构造函数在给定地址上构造对象** |

---

# 🌟 为什么需要 placement new？

**三大用途：**

---

## 1️⃣ 手动管理内存区域（内存池）

当你实现一个数据结构库（你未来的目标），例如：

* vector 的扩容
* 自定义 allocator
* 内存池 memory pool
* 特定对齐要求的对象
* 固定容量容器，如 small_vector

你需要：

* 提前申请一块大内存（如 `malloc` 或 `std::aligned_storage`）
* 在其上构造/析构 N 个对象

这种场景下，placement new 是唯一工具。

---

## 2️⃣ 在已有内存中“重新构造”对象

特别是**对象重置 / 缓冲区复用**：

```cpp
char buffer[sizeof(A)];
A* a = new (buffer) A(1);

// ...想重新构造一个新 A 对象在相同位置
a->~A();
a = new (buffer) A(2);
```

避免重复分配内存。STL 的 **emplace 系列函数**（如 `vector::emplace_back`）就是这样做的。

---

## 3️⃣ 构造在非普通内存区域的对象

例如：

* 在共享内存（shm）上构造对象
* 在文件映射的区域（mmap）上构造对象
* 在 DMA 缓冲区中构造对象
* placement new + 自定义对齐内存

操作系统和硬件驱动都经常用。

---

# 🌟 placement new 的关键点

placement new 本质其实就是呼叫构造函数：

```cpp
void* operator new(size_t, void* place) noexcept {
    return place;
}
```

它 **不分配内存，只返回 place**
然后构造函数在它上面“就地构造”对象。

---

# 🌟使用示例：最核心最典型的写法

你已经看过基本例子了，我给你一个“工业级写法”：

```cpp
#include <new>
#include <iostream>

struct Foo {
    Foo(int x) : x(x) {
        std::cout << "Construct " << x << std::endl;
    }
    ~Foo() {
        std::cout << "Destruct " << x << std::endl;
    }
    int x;
};

int main() {
    alignas(Foo) char buffer[sizeof(Foo)];

    Foo* f = new (buffer) Foo(10); // 构造对象
    std::cout << f->x << std::endl;

    f->~Foo(); // ⚠️ 必须手动析构
}
```

---

# 🌟 placement new 带来的义务

placement new **不会调用 delete，不会释放内存，也不会析构对象**
因此：

> 你必须在适当时机手动调用析构函数。

否则你会泄漏资源：

```cpp
f->~Foo();
```

为什么必须手动调用析构？

因为语法上：

## ❌ 错误：不能使用 delete

```cpp
delete f;  // 错误！！！
```

delete 做三件事：

1. 调用析构函数
2. 释放内存（通过 operator delete）
3. 清理元数据

而我们的内存 **不是 new 创建的**！
delete 不知道该如何释放。

所以 **delete 永远不能与 placement new 配对**。

---

# 🌟 STL 中的 placement new 使用场景

placement new 的使用在 STL 中极为广泛，包括：

### ✔ vector 的扩容

扩容时会：

1. 分配一块更大的内存
2. 用 **placement new** 将旧数据移动或拷贝到新位置
3. 调用旧对象的析构

源码中典型样例：

```cpp
::new (static_cast<void*>(dst)) T(std::move(src));
```

### ✔ emplace 系列函数

`vector::emplace_back`, `map::emplace`, `tuple` 等
都是在一块原始内存上直接构造对象：

```cpp
::new (pointer) T(args...);
```

### ✔ 自定义 allocator 必须使用 placement new

allocator 负责“分配内存”，但构造对象在分配的内存上用的是 placement new。

---

# 🌟 placement new 与 std::aligned_storage, std::byte 配合使用

一个工业级内存池可以这么写：

```cpp
#include <new>
#include <type_traits>

template<typename T>
class ObjectPool {
    static constexpr size_t N = 1024;
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer[N];
    bool used[N] = {false};
public:
    T* allocate() {
        for (size_t i = 0; i < N; i++) {
            if (!used[i]) {
                used[i] = true;
                return new (&buffer[i]) T(); // placement new
            }
        }
        return nullptr;
    }

    void deallocate(T* p) {
        p->~T();
        size_t i = (reinterpret_cast<char*>(p) -
                    reinterpret_cast<char*>(buffer))
                    / sizeof(T);
        used[i] = false;
    }
};
```

placement new 就是核心。

---

# 🌟 常见错误（现实中最常见的坑）

### ❌ 1. 用 placement new 构造，但忘记析构

导致对象资源泄漏（如内部 new）

### ❌ 2. placement new 构造数组类型（需要特殊语法）

数组需要构造元素，不能用普通构造函数循环替代（STL 会处理）

### ❌ 3. 与 delete 混用（危险）

因为 delete 会试图释放不属于它的内存。

---

# 🌟 placement new 和普通 new 的“底层差异”

| 操作     | 普通 new | placement new |
| ------ | ------ | ------------- |
| 内存分配   | ✔ 是    | ❌ 否           |
| 调用构造函数 | ✔ 是    | ✔ 是           |
| 释放操作   | delete | 手动析构          |
| 生命周期管理 | 自动     | 完全手动          |
| 适用场景   | 创建对象   | 在预分配内存上就地构造   |

---

# 🌟一句话总结

> **placement new = 在某块已存在的内存上调用构造函数**
> 用于：容器扩容、内存池、allocator、emplace、硬件共享区构造对象。
>
> 它给你控制权，但你也必须负责析构。

---

如果你愿意，我还可以补充：

* 📌 placement delete（定位 delete）是什么？为什么你几乎永远不会用？
* 📌 placement new 的异常安全性处理（tricky）
* 📌 在 vector 扩容流程中一步一步展示 placement new 的作用

需要吗？
