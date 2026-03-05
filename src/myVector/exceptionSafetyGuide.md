# C++ 异常安全指南

异常处理是 C++ 现代编程中处理运行时错误的核心机制。对于像 `std::vector` 这样的底层容器，实现**异常安全性**不仅仅是为了让程序不崩溃，更是要保证数据的一致性和资源不泄漏。

本文将深入讲解异常的基本概念，基本语法（与常见异常类型），四级异常安全，异常安全惯用法和有关异常的其他事项。

## 1. 什么是异常 (Exception)
**异常**是程序运行时发生的、打断正常指令流的意外事件。

在 C 语言中，我们通常通过返回值（如返回 `-1` 或 `NULL`）来表示错误。但在 C++ 中，异常机制提供了更好的替代方案：
1.  **强制处理**：错误码可以被忽略，但异常如果不捕获，会导致程序终止 (`std::terminate`)。
2.  **构造函数无返回值**：如果对象构造失败（例如内存分配失败），构造函数无法返回值，只能通过抛出异常来阻止对象被创建。
3.  **模块解耦**：正常的业务逻辑（Happy Path）和错误处理逻辑（Error Path）在代码上是分离的，可读性更高的同时也便于更好地维护。

在 C++ 中，**异常安全**（Exception Safety）是指 C++ 代码在发生异常时能否保持正确性，避免资源泄漏和未定义行为。一般而言，C++ 的异常安全等级分为四级：无保证、基本保证、强保证、不抛异常保证。（也有些分为三级，不含无保证）

## 2. 异常的基本语法 

C++ 使用 `throw` 关键字抛出异常，使用 `try...catch` 块捕获并处理异常，使用 `noexcept` 声明函数不抛出异常。

有关异常处理的头文件主要是 `<stdexcept>, <exception>`。这两个头文件各自包含的内容有所区别，在文末的其他事项中会介绍。

### 2.1 异常处理格式

以下是一个标准的异常捕获与处理格式：
```cpp
try {
    // A. 可能抛出异常的代码
    // 一旦在这里抛出异常，控制流立即跳到 catch 块
    std::vector<int> v(1000000000000); // 可能抛出 bad_alloc
} 
catch (const std::bad_alloc& e) {
    // B. 处理特定类型的异常
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
} 
catch (const std::exception& e) {
    // C. 处理所有标准库异常
    std::cerr << "Standard exception: " << e.what() << std::endl;
}
catch (...) {
    // D. 处理所有其他类型的异常（最后的防线）
    std::cerr << "Unknown exception occurred." << std::endl;
}
```

利用这样的格式，我们可以写一个最简单的异常处理代码——输入年龄，若不合理则抛异常。
```cpp
void checkAge(int age) {
    if (age < 0 || age > 150) {
        throw std::out_of_range("年龄必须在0~150之间");
    }
    std::cout << "年龄有效: " << age << std::endl;
}

int main() {
    std::cout << "请输入年龄: ";
    int age;
    std::cin >> age;
    try {
        checkAge(age);  // 抛出异常
    }
    catch (const std::out_of_range& e) {
        std::cerr << "输入错误: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "其他标准异常: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "未知异常" << std::endl;
    }
    return 0;
}
```

### 2.2 标准异常类（std::exception）
C++标准库提供了一系列异常类，都派生自 `std::exception`，定义在 `<stdexcept>, <exception>, <new>, <system_error>` 等头文件中。

### 2.2.1 运行时错误 (Runtime Errors)
指只有在程序运行时才能检测到的客观环境问题，派生自 `std::runtime_error`（`bad_alloc` 除外）。

| 异常类型 | 含义 | 典型场景 |
| :--- | :--- | :--- |
| `std::bad_alloc` | 内存分配失败 | `new` 或 `allocator::allocate` 时系统内存耗尽 |
| `std::runtime_error` | 通用运行时错误 | 无法检测到的外部错误，其他运行时错误的基类 |
| `std::overflow_error` | 算术运算上溢 | 整数运算结果超出类型表示范围（如 `bitset::to_ulong`） |
| `std::underflow_error` | 算术运算下溢 | 浮点数运算结果幅度过小无法表示 |
| `std::system_error` | 系统底层错误 (C++11) | 与操作系统交互失败，如 `std::thread` 创建失败，文件操作失败 |

### 2.2.2 逻辑错误 (Logic Errors)
指程序逻辑本身的问题，理论上可以通过修改代码避免（前置条件检查），派生自 `std::logic_error`。

| 异常类型 | 含义 | 典型场景 |
| :--- | :--- | :--- |
| `std::out_of_range` | 参数超出允许范围 | `vector::at(index)` 越界，`bitset::set` 越界 |
| `std::length_error` | 长度超出限制 | `vector` 扩容超过 `max_size()` |
| `std::invalid_argument` | 参数非法 | `std::stoi("abc")` 转换无效字符串，`bitset` 构造含非 0/1 字符 |
| `std::domain_error` | 数学定义域错误 | 参数不在函数的定义域内（如某些数学库函数） |


## 3. 异常安全等级 (Exception Safety Levels)
这是评估 C++ 代码（特别是容器库）质量的重要指标。

我们通常将安全等级分为四级（由弱到强）：

### 3.1 异常安全等级对照表

| 等级 | 核心承诺 | 表现 | 典型场景/实现模式 |
| :--- | :--- | :--- | :--- |
| **无保证**<br>(No Guarantee) | 无 | 内存可能泄漏，数据可能损坏，迭代器失效 | **不可接受**。如：只增加了 `size` 但未写入数据 |
| **基本保证**<br>(Basic Guarantee) | 不泄漏资源<br>不产生未定义行为 | 对象处于**合法但不确定**的状态（可以安全销毁或重置），所有分配的资源都能释放 | 销毁过程中部分失败，但确保已分配内存正确释放 |
| **强保证**<br>(Strong Guarantee) | 事务性安全<br>(Commit or Rollback) | 要么成功，要么**回滚**到调用前状态，就像操作从未发生一样 | **Copy-and-Swap** 策略；<br>`reallocate`：分配新内存 -> 构造 -> (失败则释放新 / 成功则释放旧) |
| **不抛出保证**<br>(Nothrow Guarantee) | 绝不失败<br>(`noexcept`) | 保证在任何情况下都不会抛出异常 | **析构函数**、**移动构造**、**Swap** |

## 4. 异常安全惯用法 (Idioms)

实现异常安全的代码有一些经典的模式和惯用法，掌握它们能大大简化代码编写。

### 4.1 RAII (资源获取即初始化)
这是 C++ 异常安全的基石。通过对象生命周期管理资源（内存、文件句柄、锁），确保即使发生异常，析构函数也能自动释放资源。

**原则**：不要手动 `new/delete`，使用智能指针；不要手动 lock/unlock，使用 `std::lock_guard`。
**示例**：
```cpp
void process() {
    // 不推荐：如果中途抛出异常，delete 不会被执行
    int* p = new int[100]; 
    doSomething(p); // 可能抛出
    delete[] p;
    // 推荐：无论是否抛出异常，ptr 析构时都会释放内存
    std::unique_ptr<int[]> ptr = std::make_unique<int[]>(100);
    doSomething(ptr.get());
}
```

也有一种流行的实践方式，是将裸指针封装在类中，通过构造/析构函数来分配/释放内存。即，将指针指向的内存的存储器与类的生存期绑定起来，使其满足 RAII。

### 4.2 Copy-and-Swap (拷贝并交换)
这是实现**强异常安全保证**（Strong Guarantee）赋值运算符的标准写法。

**原理**：利用拷贝构造函数创建临时副本（可能抛出），然后利用不抛异常的 `swap` 更新状态。如果拷贝失败，异常在进入函数体前（或参数构造时）抛出，`*this` 保持不变。
**示例**：
```cpp
class MyClass {
    int* data;
public:
    // 1. 拷贝构造（可能抛出）
    MyClass(const MyClass& other) : data(new int(*other.data)) {}
    // 2. 为了 Copy-and-Swap，我们需要一个不抛异常的 swap
    friend void swap(MyClass& first, MyClass& second) noexcept {
        using std::swap;
        swap(first.data, second.data);
    }
    // 3. 赋值运算符（强保证）
    // 注意参数是按值传递 (pass-by-value)，隐式调用了拷贝构造
    MyClass& operator=(MyClass other) {
        swap(*this, other); // 交换资源
        return *this;
    } // other 在这里析构，释放了旧的资源
};
```

## 5. 其他重要事项

### 5.1 构造函数与析构函数
**构造函数可以抛出异常**：这是报告构造失败的标准方式。但要注意，如果构造函数抛出异常，该对象的**析构函数不会被执行**（因为生命周期未开始）。因而建议，成员变量尽量使用智能指针，这样即使构造函数体抛出异常，已构造好的成员也能自动析构。

**析构函数绝不能抛出异常**：原因为如果析构函数在“栈展开”（Stack Unwinding，即异常处理过程中销毁局部对象）期间抛出异常，C++ 会直接调用 `std::terminate` 导致程序崩溃。所以析构函数应默认为 `noexcept`。如果有异常，必须在内部 catch 住并处理（如记录日志）。

### 5.2 `<exception>` 与 `<stdexcept>`
**`<exception>`**：
* 定义了所有异常的基类 `std::exception`。
* 包含语言核心支持的异常，如 `std::bad_alloc` (`new` 失败), `std::bad_cast` (`dynamic_cast` 失败)。
* 包含处理机制，如 `std::terminate`, `std::exception_ptr`。

**`<stdexcept>`**：
* 包含常用的**标准逻辑错误**和**运行时错误**类（带字符串消息）。
* 如 `std::runtime_error`, `std::logic_error`, `std::out_of_range`, `std::invalid_argument`。
* 通常我们在业务逻辑中抛出的都是这里的异常。

### 5.3 什么时候使用 `noexcept`
并不是所有不抛异常的函数都要加 `noexcept`，但在以下场景务必加上：
1.  **移动操作** (`Move Constructor`, `Move Assignment`)：
    `std::vector` 在扩容时，如果元素类型的移动构造是 `noexcept`，它会使用移动（高效）；否则为了保证强异常安全，它会使用拷贝（低效）。
2.  **析构函数**：默认就是，但显式写上也无妨。
3.  **Swap 函数**：用于实现强保证的基础。

### 5.4 异常与效率
关于 C++ 异常的性能开销，现代编译器（基于 Table-Driven 机制）通常遵循 **Zero-cost abstraction** 原则（针对正常路径）：

1.  **Happy Path (不抛异常) ≈ 零开销**：
    只要不抛出异常，进入 `try` 块几乎没有运行时性能损耗。不需要像返回错误码那样每步都进行 `if (err)` 检查，这对 CPU 分支预测非常友好。
2.  **Sad Path (抛出异常) = 代价高昂**：
    一旦抛出异常，运行时系统需要查找异常表、进行栈展开（Stack Unwinding）、调用沿途对象的析构函数。这通常比普通函数返回慢几个数量级。
    **准则**：只能用异常处理**真正的异常**（如硬盘满了、网络断了），决不能用异常来控制业务流程（如循环结束、查找元素未找到）。
3.  **二进制体积增加**：
    为了支持栈展开，编译器需要在可执行文件中生成静态的异常表。即使不抛出异常，这也会导致程序体积增大（通常 10%~20%）。
4.  **阻碍优化**：
    如果函数可能抛出异常，编译器必须保守地生成代码以确保异常发生时状态正确。标记 `noexcept` 可以让编译器放手进行更激进的优化。