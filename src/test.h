#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstdlib> // for rand()
#include <ctime>   // for time()
#include "myVector/myVector.h"

// 自定义分配器，用于测试 allocator 相关功能
template <typename T>
struct DebugAllocator {
    using value_type = T;

    DebugAllocator() = default;
    template <typename U> DebugAllocator(const DebugAllocator<U>&) {}

    T* allocate(std::size_t n) {
        std::cout << "[Alloc] Allocating " << n << " elements (" << n * sizeof(T) << " bytes)\n";
        return std::allocator<T>{}.allocate(n);
    }

    void deallocate(T* p, std::size_t n) {
        std::cout << "[Alloc] Deallocating " << n << " elements\n";
        std::allocator<T>{}.deallocate(p, n);
    }
};

template <typename T, typename U>
bool operator==(const DebugAllocator<T>&, const DebugAllocator<U>&) { return true; }

template <typename T, typename U>
bool operator!=(const DebugAllocator<T>&, const DebugAllocator<U>&) { return false; }

// ==========================================
// 性能测试辅助结构：HeavyPOD
// 这里定义一个较大的 Trivially Copyable 结构体
// ==========================================
struct HeavyPOD {
    long data[8]; // 64 bytes
    // 它是 trivial 的，因为没有自定义构造/析构/赋值

    HeavyPOD() = default;
    // 添加一个从 int 构造的构造函数，以便在测试循环中使用 static_cast<T>(rand())
    // 这不会破坏 is_trivially_copyable 属性
    HeavyPOD(int v) {
        for(int i=0; i<8; ++i) data[i] = v;
    }
};

// 用于测试生命周期和移动语义的辅助类
struct Obj {
    std::string name;
    int id;

    Obj(std::string n, int i) : name(std::move(n)), id(i) {
        std::cout << "  [Ctor] " << name << "\n";
    }
    
    Obj(const Obj& other) : name(other.name), id(other.id) {
        std::cout << "  [Copy] " << name << "\n";
    }

    Obj(Obj&& other) noexcept : name(std::move(other.name)), id(other.id) {
        std::cout << "  [Move] " << name << "\n";
        other.name = "(moved)";
    }

    Obj& operator=(const Obj& other) {
        if (this != &other) {
            name = other.name;
            id = other.id;
            std::cout << "  [CopyAssign] " << name << "\n";
        }
        return *this;
    }

    Obj& operator=(Obj&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            id = other.id;
            other.name = "(moved)";
            std::cout << "  [MoveAssign] " << name << "\n";
        }
        return *this;
    }

    ~Obj() {
        if (name != "(moved)") { // 减少一点输出噪音
            std::cout << "  [Dtor] " << name << "\n";
        }
    }
};

class mydsTest {
private:
    void print_separator(const std::string& title) {
        std::cout << "\n=== " << title << " ===\n";
    }

    template <typename T>
    void perform_pressure_test_for_type(size_t element_count, const std::string& type_name, int runs = 1) {
        long long std_duration_ms = 0;
        long long my_duration_ms = 0;

        // Test std::vector
        srand(unsigned(time(nullptr))); // 设置随机种子
        for (int r = 0; r < runs; ++r)
        {
            auto start = std::chrono::high_resolution_clock::now();
            std::vector<T> v;
            for (size_t i = 0; i < element_count; ++i) {
                v.push_back(static_cast<T>(rand())); // 使用随机数填充，避免编译器优化掉 push_back
            }
            auto end = std::chrono::high_resolution_clock::now();
            std_duration_ms += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

        // Test myVector
        for (int r = 0; r < runs; ++r)
        {
            auto start = std::chrono::high_resolution_clock::now();
            myVector<T> v;
            for (size_t i = 0; i < element_count; ++i) {
                v.push_back(static_cast<T>(rand())); // 使用随机数填充，避免编译器优化掉 push_back
            }
            auto end = std::chrono::high_resolution_clock::now();
            my_duration_ms += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

        std_duration_ms /= runs;
        my_duration_ms /= runs;
        std::cout << "| " << std::left << std::setw(12) << type_name 
                  << "| " << std::right << std::setw(15) << element_count 
                  << "| " << std::setw(9) << std_duration_ms << " ms"
                  << "| " << std::setw(9) << my_duration_ms << " ms"
                  << "| " << std::setw(9) << std::fixed << std::setprecision(2) << (double)my_duration_ms / std_duration_ms << "x |\n";
    }

public:
    void test1_BasicPushAndAccess() {
        print_separator("测试 1: 基本操作功能 - push_back 和 operator[]");
        myVector<int> v1;
        v1.push_back(10);
        v1.push_back(20);
        v1.push_back(30);

        std::cout << "v1 大小: " << v1.size() << ", 容量: " << v1.capacity() << "\n";
        std::cout << "v1 内容: ";
        for (size_t i = 0; i < v1.size(); ++i) {
            std::cout << v1[i] << " ";
        }
        std::cout << "\n";
    }

    void test2_ObjectLifecycle() {
        print_separator("测试 2: 对象生命周期 - 构造、复制、移动、销毁");
        {
            myVector<Obj> v2;
            std::cout << "-- 执行 emplace_back(\"Alice\", 1) --\n";
            v2.emplace_back("Alice", 1);
            
            std::cout << "-- 执行 push_back(Obj(\"Bob\", 2)) --\n";
            v2.push_back(Obj("Bob", 2));

            std::cout << "-- 执行 reserve(10) (应触发移动构造) --\n";
            v2.reserve(10);
            
            std::cout << "v2 大小: " << v2.size() << ", 容量: " << v2.capacity() << "\n";
        } // v2 destructs here
    }

    void test3_CopyAndMove() {
        print_separator("测试 3: 拷贝和移动语义");
        myVector<int> v3;
        v3.push_back(100);
        v3.push_back(200);

        std::cout << "-- 复制 v4 = v3 --\n";
        myVector<int> v4 = v3;
        std::cout << "v4[0]: " << v4[0] << "\n";
        v4[0] = 999;
        std::cout << "v4 修改后. v3[0]: " << v3[0] << " (应为 100)\n";

        std::cout << "-- 移动 v5 = std::move(v3) --\n";
        myVector<int> v5 = std::move(v3);
        std::cout << "v5 大小: " << v5.size() << "\n";
        std::cout << "v3 大小: " << v3.size() << " (应为 0)\n";
    }

    void test4_ExceptionSafety() {
        print_separator("测试 4: 异常安全");
        std::cout << "异常安全性主要依赖逻辑验证。\n";
        std::cout << "当前实现使用了 std::move_if_noexcept。\n";
    }

    void test5_Insert() {
        print_separator("测试 5: 插入操作 (Insert)");
        myVector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(4);
        
        std::cout << "-- 初始状态: 1 2 4 --\n";
        
        std::cout << "-- 执行 insert(begin()+2, 3) --\n";
        auto it = v.insert(v.begin() + 2, 3);
        std::cout << "返回值: " << *it << "\n";
        
        std::cout << "-- 执行 insert(begin(), 0) --\n";
        v.insert(v.begin(), 0);

        std::cout << "-- 执行 insert(end(), 5) --\n";
        v.insert(v.end(), 5);

        std::cout << "v 内容: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(预期: 0 1 2 3 4 5)\n";
    }

    void test6_Erase() {
        print_separator("测试 6: 删除操作 (Erase)");
        myVector<int> v;
        // Init: 0 1 2 3 4 5
        for(int i=0; i<=5; ++i) v.push_back(i);

        std::cout << "-- 执行 erase(begin()) -> 删除 0 --\n";
        v.erase(v.begin());
        
        std::cout << "-- 执行 erase(end()-1) -> 删除 5 --\n";
        v.erase(v.end() - 1);

        std::cout << "-- 执行 erase(begin()+1, begin()+3) -> 删除 2, 3 --\n";
        // Current: 1 2 3 4
        // Index:   0 1 2 3
        v.erase(v.begin() + 1, v.begin() + 3);

        std::cout << "v 内容: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(预期: 1 4)\n";
    }

    void test7_Resize() {
        print_separator("测试 7: 大小调整 (Resize)");
        myVector<int> v;
        v.push_back(1);
        v.push_back(4);

        // Current v: 1 4 (size 2)
        std::cout << "-- 执行 resize(5, 10) -> 增长并填充 10 --\n";
        v.resize(5, 10);
        std::cout << "v 大小: " << v.size() << ", 容量: " << v.capacity() << "\n";
        std::cout << "v 内容: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(预期: 1 4 10 10 10)\n";

        std::cout << "-- 执行 resize(3) -> 缩小 --\n";
        v.resize(3);
        std::cout << "v 大小: " << v.size() << "\n";
        std::cout << "v 内容: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(预期: 1 4 10)\n";

        std::cout << "-- 执行 resize(6) -> 增长并默认构造 (0) --\n";
        v.resize(6);
        std::cout << "v 内容: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(预期: 1 4 10 0 0 0)\n";
    }

    void test8_CustomAllocator() {
        print_separator("测试 8: 自定义分配器检查");
        // 使用自定义分配器声明 Vector
        myVector<int, DebugAllocator<int>> v_custom;
        std::cout << "Push 1, 2, 3 (预期会有分配信息)\n";
        v_custom.push_back(1);
        v_custom.push_back(2);
        v_custom.push_back(3);

        std::cout << "Vector 析构 (预期会有释放信息)\n";
        // main 结束时，v_custom 会析构，应该打印 deallocate 消息
    }

    void test9_Performance(int N = 100000000, int runs = 3) {
        print_separator("测试 9: 高压性能测试");
        std::cout << "正在运行性能测试...\n";
        std::cout << "| 类型        | 数量           | std::vector |  myVector   | 比率       |\n";
        std::cout << "|-------------|----------------|-------------|-------------|------------|\n";
        
        perform_pressure_test_for_type<int>(N, "int", runs);
        perform_pressure_test_for_type<long long>(N, "long long", runs);
        perform_pressure_test_for_type<float>(N, "float", runs);
        perform_pressure_test_for_type<double>(N, "double", runs);
        
        // 当元素变大时，memcpy 对比逐个构造的优势应该更明显, 或者当 std::vector 可能还没为此类优化到最佳时
        perform_pressure_test_for_type<HeavyPOD>(N / 2, "HeavyPOD 64B", runs); 
    }

    // 可选：运行所有测试
    void runAllTests() {
        try {
            test1_BasicPushAndAccess();
            test2_ObjectLifecycle();
            test3_CopyAndMove();
            test4_ExceptionSafety();
            test5_Insert();
            test6_Erase();
            test7_Resize();
            test8_CustomAllocator();
            test9_Performance();
        } catch (const std::exception& e) {
            std::cerr << "测试因异常失败: " << e.what() << "\n";
        }
    }
};

#endif // TEST_H
