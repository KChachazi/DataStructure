# Updates

续```part2-1.md```继续写

前面文章简要介绍了内存管理的基础内容，主要包含栈与堆的内存分配，`new/delete`与`malloc/free`，**RAII思想**，三/五法则和零法则。

接下来将结束内存管理部分，介绍移动语义与右值引用（C++11+）和内存泄漏检测与防范。

# 面向对象与内存管理

## 移动语义与右值引用

C++11 引入的移动语义用于解决传统拷贝操作的高成本问题。在大量临时对象、容器扩容、对象传递等场景中，通过“移动”资源而非“复制”资源，可以显著提升性能。

### 左值与右值

首先我们要明确，左值和右值表示的是表达式的属性，一个对象(变量)可以构成左值表达式和右值表达式。换句话说，对象是实体，而左右值表示使用方式。

左值（lvalue）：具有标识符、有固定内存地址的表达式，可以取地址，如变量 `x`。

右值（rvalue）：临时对象、字面量或即将被销毁的表达式，不能取地址，如 42、`std::string("abc")`、函数返回临时对象。

右值通常代表“即将消亡的对象”，其资源可被安全接管。

用一个形象的比喻：左值即水壶，右值为一次性纸杯。

#### 为什么要使用右值引用/左值的问题

在C++11之前，很多C++程序里存在大量的临时对象，主要出现在如下场景：函数返回值、用户自定义类型经过一些计算后产生的临时对象、值传递的形参等等。

这些临时对象随着生命周期的结束还会再此调用一次析构函数。在操作次数增多或临时变量较大的情况下，这种操作会极大地占用性能，降低效率。

为了解决上面提到的问题，C++11开始引入了移动语义和右值引用。

### 移动语义

移动语义依赖两个核心机制，分别为右值引用（T&&）和显式移动（`std::move`）

#### 右值引用（T&&）

右值引用可以绑定到右值，从而允许在构造过程中“窃取”其内部资源。如：

```cpp
int &&x = 10;
```

右值引用不等于“右值本身”，但它绑定的是一种“允许被移动”的对象。

#### `std::move`

std::move(x) 的作用是将 x 转换为右值引用，表示“你可以移动我”。
它本身不做移动操作，**真正执行移动的是移动构造/移动赋值函数**。

```cpp
std::string str = "Hello, World!";
std::cout << "Original string: " << str << '\n';
// 使用 std::move 将 str 换为右值引用
std::string t = std::move(str);
std::cout << "Moved string: " << t << '\n';
// str 的状态未定义，通常为空
std::cout << "Original string after move: " << str <<'\n';
```

上面的代码段运行后输出：
```text
Original string: Hello, World!
Moved string: Hello, World!
Original string after move:
```

此外，对于类而言来看下面的实例：
```cpp
class Sample {
public:
    Sample() : data(new int[100]) {
        std::cout << "Constructor called\n";
    }

    // 移动构造函数
    Sample(Sample&& other) noexcept : data(other.data) {
        other.data = nullptr; // 将其他对象的数据指针置为空，防止双重释放
        std::cout << "Move constructor called\n";
    }

    ~Sample() {
        delete[] data;
        std::cout << "Destructor called\n";
    }

    void display(std::string msg) const {
        std::cout << msg;
        if (data) {
            std::cout << "Data is " << data << '\n';
        } else {
            std::cout << "Data is null\n";
        }
    }
private:
    int* data;
};
```

对上面的`Sample`类执行下面的代码：
```cpp
Sample obj1;
Sample obj2 = std::move(obj1);
obj1.display("After move, obj1: ");
obj2.display("After move, obj2: ");
return 0;
```

得到输出如下：
```text
Constructor called
Move constructor called
After move, obj1: Data is null
After move, obj2: Data is 0x2240d42c590
Destructor called
Destructor called
```

可以看出，通过移动赋值函数，我们成功将`obj1`中的`int*`数组指针直接移交给了`obj2`。

#### 移动构造函数与移动赋值运算符

若类管理资源（如动态内存），应手动定义移动构造与移动赋值。实际上前面的例子已经包含本部分内容，下面仅再举一个实例：

```cpp
class Buffer {
private:
    char* data;
    std::size_t size;

public:
    // 移动构造
    Buffer(Buffer&& other) noexcept
        : data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0;
    }

    // 移动赋值
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }
};
```

推荐在移动操作上标记 noexcept，使容器在重排时优先选择移动而不是拷贝。

#### 移动语义的应用场景

正如虚继承是为了解决棱形继承的问题一样，接下来在讨论移动语义应用场景之前，我们更需要先看懂移动语义解决问题的出发点，或者说它期望解决什么问题。

##### 移动语义到底解决了什么问题

移动语义首先通过区分左值和右值的概念，然后尝试在对象之间实现资源的转移/移动，因而移动语义解决的是临时对象、即将销毁对象的高效资源转移问题。

比如下面一个实例：
```cpp
template<typename T>
vector<T> createVector(size_t size, const T& initialValue) {
    vector<T> vec(size, initialValue);
    return vec;
}
```

对于没有移动语义的理解方式为，return一个对象，触发拷贝构造，深拷贝整块堆内存；而引入移动语义后，v是即将销毁的对象，编译器可选择移动构造，只需交换指针，复杂度`O(1)`。

##### 应用场景

移动语义的应用场景就是需要进行临时对象、即将销毁对象的高效资源转移的场景。

返回局部对象
编译器会自动执行 NRVO 或移动。

容器扩容或重新排序
std::vector 会优先移动元素。

传递临时对象
临时值会自动调用移动构造。

存储大量资源对象
如大矩阵、大字符串、文件句柄等。

#### 移动语义的注意事项

被移动对象需保持“有效但未指定”的状态，例如：

指针设为 nullptr

数量设为 0

不要在移动后继续依赖被移动对象的数据。

小结

移动语义提供了一种高效的资源转移方式，避免不必要的复制。右值引用与 std::move 是移动语义的基础，配合移动构造与移动赋值，可以大幅提升程序性能。

## 内存泄漏检测与防范（Valgrind / Sanitizer 工具）

在 C++ 中，内存泄漏通常指：

程序分配了内存，但未能正确释放；

对象生命周期管理不正确；

有泄漏但未表现为崩溃，需要专门工具检测。

两类主流工具用于检测内存错误：

Valgrind（Memcheck）：Linux 环境中最强大的内存分析工具。

Sanitizers（AddressSanitizer / LeakSanitizer）：编译器内置，运行速度较快。

### Valgrind（Memcheck）

Valgrind 会模拟程序的执行环境，检测以下问题：

内存泄漏

访问已释放的内存（use-after-free）

使用未初始化的内存

堆越界访问

重复释放（double free）

运行方式

编译需保留调试信息：

g++ -g -O0 example.cpp -o example


使用 Valgrind：

valgrind --leak-check=full --show-leak-kinds=all ./example


常见泄漏分类：

definitely lost：真正泄漏，必须修复

indirectly lost：被泄漏对象引用的数据

possibly lost：可能泄漏

still reachable：仍然可达（未必是问题）

例如：

int main() {
    int* p = new int[10]; // 忘记 delete[]
    return 0;
}


Valgrind 会报告：
definitely lost: 40 bytes

### AddressSanitizer / LeakSanitizer

Sanitizers 由编译器注入检测代码，在执行时检查内存错误。

常用选项：

g++ -g -O1 -fsanitize=address -fno-omit-frame-pointer example.cpp -o example
./example


可以检测：

堆越界

栈越界

use-after-free

内存泄漏（可开启 LSan）

LeakSanitizer 开启方式：

g++ -fsanitize=leak example.cpp -o example
./example


Sanitizers 的优点：

检测速度快（通常为普通运行的 2×）

报告信息详细，包括完整堆栈

### 示例：检测泄漏

示例代码：

void leak() {
    int* p = new int[100];
}

int main() {
    leak();
}


在 AddressSanitizer 下运行，会输出类似：

LEAK SUMMARY:
   definitely lost: 400 bytes in 1 blocks

### 防止内存泄漏的常见方法

使用智能指针 (unique_ptr, shared_ptr) 管理资源
避免手写 new 和 delete。

使用 RAII（资源获取即初始化）机制
将资源封装在对象生命周期内自动管理。

启用编译器警告（-Wall -Wextra）
能发现潜在错误。

在开发阶段开启 Sanitizers
在 CI/CD 配置中加入带 sanitizer 的构建。

使用容器代替裸数组
如 std::vector、std::string。

小结

Valgrind 适合离线、精细调试；

AddressSanitizer 适合日常开发与 CI；

配合智能指针、RAII，可有效避免泄漏；

在高质量 C++ 项目中，应主动启用这两类工具对代码进行检测。