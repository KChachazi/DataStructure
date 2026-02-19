# 概述

C++ vector 是一种序列容器，可以称为变长数组。下面可以给出一个强有力的定义：vector 是一个**连续内存、动态增长、拥有值语义、负责对象生命周期的通用序列容器**。

# 功能目标

从抽象层面来看，vector 的功能目标即定义中的几个限制词，下文将要一一介绍。

## 连续内存

vector 与 list/deque 的根本区别在于，其要管理的是一段连续的对象序列。在内存空间中，表现为 `&v[i] + 1 == &v[i+1]`。

连续内存的要求使得 vector 可以直接与 `C API / SIMD / cache` 进行交互，带来极大的使用便利；只是直接导致在 vector 的空间中不能随便插空，在中间的插入/删除操作的时间复杂度必定为 `O(n)`，而且扩容时只能“举家搬迁”。

## 负责对象生命周期

vector<T> 不是仅仅对 T 类型的对象进行简单封装，而更是一个对象生命周期管理器。vector 必须负责分配原始内存，在内存中构造对象，在适当时机析构对象，最后释放内存。

## 拥有值语义

Vector 容器存储的是对象的“值”而非“引用”或“指针”。当你将一个对象放入 vector 时，vector 会**拷贝**（或移动）一份该对象到自己的内存空间中。这意味着：
* 独立性：容器内元素的修改不会影响到外部原始对象。
* 自包含：Vector 销毁时，其内部持有的对象也会随之销毁，无需用户手动管理内存。
* 拷贝代价：存入和获取对象可能涉及拷贝构造，对于重量级对象，应利用“移动语义”或 `emplace` 系列接口来优化。

## 通用序列容器

Vector 是一个模板类 `vector<T>`，意味着它可以存储任意类型的对象（只要该对象满足特定条件，如可拷贝或可移动）。
* 同质性：同一个 vector 实例只能存储一种类型的对象。
* 序列性：元素按照插入顺序严格线性排列，支持按索引（index）直接定位。序列容器的核心特征是：用户可以控制元素存储的位置（比如插在开头、中间或结尾）。

## 提供支持STL生态的接口

总的来说，vector 能做的是高效随机访问，cache-friendly 遍历，尾部插入高效，与 C 接口无缝交互；vector 明确不能保证中间插入高效，指针/迭代器稳定性，泛型下不抛异常。

# 实现

vector 有一个大体框架（如下），`capacity` 管理的是 vector 的总内存大小/最大可容纳的元素数量，`size` 管理的是 vector 已构造的元素数量。

```
┌─────────────────┐
│    allocated    │  ← capacity
│ ┌─────────────┐ │
│ │ constructed │ │  ← size
│ └─────────────┘ │
└─────────────────┘
```

## 第一阶段：实现最小可行 vector：intVector

详略，在实现 intVector 的过程中先初步了解 vector 的基本功能设计与实现。

## 第二阶段：实现主要接口

### 1. 构造与析构 (Constructor / Destructor)
负责对象的创建、拷贝、移动及销毁。
*   `myVector()`: 默认构造函数，创建一个空容器。
*   `~myVector()`: 析构函数，释放所有元素和内存。
*   `myVector(const myVector&)`: 拷贝构造函数（深拷贝）。
*   `myVector(myVector&&)`: 移动构造函数（资源接管）。
*   `operator=(const myVector&)`: 拷贝赋值运算符。
*   `operator=(myVector&&)`: 移动赋值运算符。

### 2. 容量操作 (Capacity)
查询容器状态或手动管理容量。
*   `size()`: 返回当前元素个数。
*   `capacity()`: 返回当前已分配的内存可容纳元素总数。
*   `empty()`: 判断容器是否为空。
*   `resize(n)`: 改变容器大小（可能引起扩容或截断）。
*   `reserve(n)`: 预分配内存，避免多次扩容。

### 3. 元素访问 (Element Access)
访问容器中的具体元素。
*   `operator[]`: 下标访问（不检查越界）。
*   `at()`: 安全下标访问（越界抛异常）。
*   `front()`: 访问第一个元素。
*   `back()`: 访问最后一个元素。

### 4. 迭代器 (Iterators)
支持 STL 算法的随机访问迭代器。
*   `begin()`, `end()`: 返回指向首尾的迭代器。
*   `cbegin()`, `cend()`: 返回只读迭代器（实现中暂未显式区分，通过 const 重载支持）。

### 5. 修改器 (Modifiers)
修改容器内容，涉及元素的增加、删除和赋值。
*   `push_back(val)`: 尾部追加元素（拷贝/移动）。
*   `pop_back()`: 弹出尾部元素。
*   `emplace_back(args...)`: 尾部原地构造元素。
*   `insert(pos, val)`: 在指定位置插入元素。
*   `erase(pos)`: 删除指定位置元素。
*   `erase(first, last)`: 删除指定区间的元素。
*   `clear()`: 清空所有元素（不释放内存）。

### 6. 内部工具 (Internal Tools)
不对外公开，负责底层资源管理实现。
*   `allocate(n)`: 分配原始内存。
*   `construct_at(...)`: 在指定位置原地构造对象（Placement New）。
*   `destroy_at(index)`: 显式调用对象析构函数。
*   `destroy_range(from, to)`: 批量销毁对象。
*   `reallocate(new_cap)`: 核心扩容逻辑（申请新空间 -> 移动旧数据 -> 释放旧空间）。

### 1. 构造与析构 (Constructor / Destructor)

构造与析构是容器资源管理（RAII）的核心。在实现中，必须严格区分 **内存分配** 与 **对象构造**。

#### 1.1 默认构造与析构
*   `myVector();`
*   `~myVector();`

*   默认构造：初始化为空状态 (`_data = nullptr`, `_size = 0`, `_capacity = 0`)，不进行任何内存分配。
*   析构函数：
    1.  调用 `destroy_range(0, _size)`：调用每个现有元素的析构函数。
    2.  调用 `operator delete[](_data)`：释放原始内存块。

#### 1.2 拷贝控制 (Deep Copy)
*   `myVector(const myVector& other);`
*   `myVector& operator=(const myVector& other);`

拷贝构造函数和赋值运算符实现**深拷贝**，确保容器拥有独立的资源副本。
*   实现逻辑：
    1.  根据源对象 (`other`) 的 `capacity` 分配新的内存块。
    2.  遍历源对象数据，调用 `construct_at` (placement new) 将元素拷贝到新内存中。
    3.  复制 `_size` 和 `_capacity` 状态。
*   赋值运算符额外细节：需处理 **自赋值检查** (`this != &other`)，通常采用 `Clear -> Allocate -> Copy` 或 Copy-and-Swap 策略。

#### 1.3 移动语义 (Move Semantics)
*   `myVector(myVector&& other) noexcept;`
*   `myVector& operator=(myVector&& other) noexcept;`

移动构造和赋值实现**资源所有权转移**，是提升性能的关键。
*   移动构造：
    *   实现：直接接管源对象的 `_data` 指针及大小状态。
    *   置空：必须将源对象 (`other`) 重置为 `_data = nullptr`, `_size = 0`，防止源对象析构时释放已被转移的内存。
*   移动赋值 (`noexcept`)：
    *   在接管资源前，先清理 (`clear` + `delete`) 自身持有的旧资源，然后执行移动逻辑。

> **关键点**：移动操作被标记为 `noexcept`。这告诉编译器该操作不会抛出异常，使得 `vector` 在扩容（`reallocate`）时可以安全地使用 `std::move_if_noexcept` 迁移元素，而不是回退到昂贵的拷贝构造。

### 2. 容量操作 (Capacity)
管理容器的大小和内存分配策略。

#### 2.1 状态查询
*   `size_t size() const noexcept;`
*   `size_t capacity() const noexcept;`
*   `bool empty() const noexcept;`

*   实现：直接返回内部成员 `_size` 和 `_capacity`，`empty()` 仅需检查 `_size == 0`。这些操作必须是 `noexcept`。

#### 2.2 大小调整 (Resize)
*   `void resize(size_t newSize);`
*   `void resize(size_t newSize, const T& value);`

*   收缩 (`newSize < _size`)：
    *   调用 `destroy_range(newSize, _size)` 销毁多余元素。
    *   关键：仅销毁对象，不释放/不缩小内存 (`capacity` 保持不变)，符合 vector "易增难减" 的特性。
*   扩张 (`newSize > _size`)：
    *   检查扩容：若 `newSize > _capacity`，触发 `reallocate`（通常按倍增策略，或至少扩容到 `newSize`）。
    *   填充元素：调用 `construct_at` 在 `[_size, newSize)` 区间内构造对象（默认构造或拷贝指定值）。
*   更新大小：操作完成后更新 `_size = newSize`。

### 3. 元素访问 (Element Access)
提供对内部元素的读写访问。这是 vector "连续内存" 特性的直接体现——支持 O(1) 随机访问。

#### 3.1 下标访问
*   `T& operator[](size_t index);`
*   `const T& operator[](size_t index) const;`
*   `T& at(size_t index);`
*   `const T& at(size_t index) const;`

*   unchecked access (`operator[]`)：直接返回 `_data[index]`。追求极致性能，不检查越界。
*   checked access (`at`)：
    *   检查 `if (index >= _size)`。
    *   越界则抛出 `std::out_of_range` 异常。

#### 3.2 首尾访问
*   `T& front();` / `const T& front() const;`
*   `T& back();` / `const T& back() const;`

*   实现：分别返回 `_data[0]` 和 `_data[_size - 1]`。
*   注意：在空容器上调用这些函数是**未定义行为 (Undefined Behavior)**。STL 标准为了性能不强制在此处做 `empty()` 检查（通常在 Debug 模式下断言）。

### 4. 迭代器 (Iterators)
在当前实现中，`vector` 的迭代器直接使用**原生指针 (`T*`)**，天然支持随机访问（Random Access）。

*   `iterator begin() noexcept;` / `iterator end() noexcept;`
*   `const_iterator begin() const noexcept;` / `const_iterator end() const noexcept;`

*   实现：
    *   `begin()` 返回 `_data`。
    *   `end()` 返回 `_data + _size`（指向最后一个元素的*下一个*位置）。
    *   原生指针支持 `++`, `--`, `+n`, `-n`, `[]` 等操作，完全满足 STL 随机访问迭代器的要求。

### 5. 修改器 (Modifiers)
修改容器内容，涉及元素的增加、删除和赋值。

#### 5.1 插入与追加
*   `void push_back(const T& value);`
*   `template <typename ... Args> void emplace_back(Args&& ... args);`
*   `iterator insert(const_iterator pos, const T& value);`
*   `iterator insert(const_iterator pos, T&& value);`

*   `push_back`: 标准的尾部追加，若空间不足触发自动扩容。
*   `emplace_back`:
    *   优势: 直接在尾部原地构造对象，避免了构造临时对象再移动/拷贝的开销。
    *   实现: 使用 `std::forward` 完美转发参数给构造函数 (Perfect Forwarding)。
*   `insert`:
    *   在特定位置插入元素，返回指向新插入元素的迭代器。
    *   Iterator Invalidation: 若触发扩容，所有迭代器失效；否则插入点之后的迭代器失效。
    *   实现细节: 需先移动插入点后的元素，再在空位构造新对象。

#### 5.2 删除与清空
*   `void pop_back();`
*   `iterator erase(const_iterator pos);`
*   `iterator erase(const_iterator first, const_iterator last);`
*   `void clear();`

*   `pop_back`: 销毁最后一个元素并减少 `_size`。不释放内存。
*   `erase`:
    *   将被删除区间*之后*的元素向前移动覆盖被删除位置（使用移动赋值）。
    *   销毁末尾多余的元素。
    *   返回指向被删除元素之后位置的迭代器。
*   `clear`: 销毁所有元素 (`destroy_range`)，重置 `_size = 0`，但**保留** `capacity`（不释放内存）。

#### 5.3 内存预留
*   `void reserve(size_t newCapacity);`
*   若 `newCapacity > _capacity`，则触发 `reallocate`。否则什么都不做。常用于已知数据量时提前分配，减少扩容开销。

### 6. 内部工具 (Internal Tools)
负责底层的内存管理与对象生命周期控制，也是 `myVector` 与普通数组最本质的区别。

| 阶段 | 函数 | 职责 | 状态变化 (Memory/Object) |
| :--- | :--- | :--- | :--- |
| **1. 分配** | `allocate(n)` | 仅申请原始内存 | Memory: Yes / Object: No |
| **2. 构造** | `construct_at(i, ...)` | 在已分配内存上构造对象 | Memory: Yes / Object: Yes |
| **3. 销毁** | `destroy_at(i)` | 调用析构函数 | Memory: Yes / Object: No |
| **4. 释放** | `operator delete[]` | 归还原始内存 | Memory: No / Object: No |

### 关键实现细节
*   `allocate(n)`：
    *   使用 `operator new[](bytes)` 分配原始字节流，**不调用**任何构造函数。
    *   这是 vector 将内存分配与对象构造解耦的关键。
*   `construct_at(index, args...)`：
    *   使用 **Placement New**: `new (addr) T(args...)`。
    *   在指定内存地址手动调用构造函数。
*   `reallocate(new_capacity)`：
    *   异常安全 (Strong Guarantee)：
        1.  先 `allocate` 新内存块。
        2.  使用 `std::move_if_noexcept` 将旧元素迁移到新内存（若移动构造可能抛异常且非必要，则退化为拷贝，保证数据不丢失）。
        3.  迁移成功后，才 `destroy` 旧对象并 `delete` 旧内存。
        4.  更新 `_data` 和 `_capacity`。
    *   若过程中抛出异常（如内存耗尽），旧数据保持不变，函数抛出异常，容器状态回滚。

## 第三阶段：支持自定义分配器

### 7. 内部工具迭代

这是 `myVector` 最核心的部分。标准库容器的精髓在于**将内存分配与对象构造分离**。在这个过程中，我们经历了两个阶段的演进。

### 7.1 原始内存管理
在引入 Allocator 之前，我们直接使用 C++ 的底层原语来管理内存。这是理解 C++ 内存模型的必经之路。

| 操作 | 代码 | 作用 |
| :--- | :--- | :--- |
| **分配** | `::operator new[](bytes)` | 仅向操作系统申请原始内存块，**不调用构造函数**。 |
| **构造** | `new (ptr) T(args...)` | **Placement New**。在已分配的 `ptr` 地址上直接构造对象。 |
| **销毁** | `ptr->~T()` | 显式调用析构函数，对象生命周期结束，但内存仍保留。 |
| **释放** | `::operator delete[](ptr)` | 将原始内存归还给操作系统。 |

**问题**：这种内存管理方式将容器与堆内存强耦合，如果想让 vector 使用共享内存、栈内存或内存池，这种硬编码的方式无法支持。

### 7.2 分配器抽象
为了让容器更加通用，C++ STL 中引入了 `Allocator` 概念作为可以更加自由设计的内存管理工具。`myVector<T, Alloc>` 将不再直接操作内存，而是委托给 `Alloc` 对象。

**标准接口**方面，我们不直接调用 `Alloc` 的成员函数，而是通过 `std::allocator_traits<Alloc>` 这个代理层进行调用，这允许 Allocator 只要实现最少量的接口即可工作。

| 抽象操作 | 原始实现 | 分配器实现 (`traits = std::allocator_traits<Alloc>`) |
| :--- | :--- | :--- |
| **定义** | `myVector<T>` | `myVector<T, Alloc = std::allocator<T>>` |
| **成员** | 无 | `[[no_unique_address]] Alloc _alloc;(C++20+)` \\ `Alloc _alloc;(other)` |
| **分配** | `operator new[](n * sizeof(T))` | `traits::allocate(_alloc, n)` |
| **构造** | `new(p) T(val)` | `traits::construct(_alloc, p, val)` |
| **销毁** | `p->~T()` | `traits::destroy(_alloc, p)` |
| **释放** | `operator delete[](p)` | `traits::deallocate(_alloc, p, n)` |

如此，`myVector` 成功将内存管理与逻辑管理进行解耦。具体地，内存来源完全由 `Alloc` 模板参数决定，而 `myVector` 只需要对内存上的对象进行管理。

### 7.3 异常安全
在 `reallocate` 扩容逻辑中，我们需要保证**强异常安全 (Strong Exception Guarantee)**：只要扩容过程中抛出异常（如内存不足或拷贝构造失败），vector 原有数据必须完好无损。

例如在 `reallocate` 中，我们要依次执行下面的步骤：分配内存、尝试构造（若失败则先回滚后抛异常）、（若构造成功）释放旧资源、更新指针。
