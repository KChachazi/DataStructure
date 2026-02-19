#include <iostream>
#include <string>
#include <memory>
#include "myVector/myVector.h"

// 测试文件，包含各种测试用例来验证 myVector 的功能和正确性

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

void print_separator(const std::string& title) {
    std::cout << "\n=== " << title << " ===\n";
}

int main() {
    try {
        print_separator("Test 1: Basic Push & Access");
        myVector<int> v1;
        v1.push_back(10);
        v1.push_back(20);
        v1.push_back(30);

        std::cout << "v1 size: " << v1.size() << ", cap: " << v1.capacity() << "\n";
        std::cout << "v1: ";
        for (size_t i = 0; i < v1.size(); ++i) {
            std::cout << v1[i] << " ";
        }
        std::cout << "\n";

        print_separator("Test 2: Object Lifecycle & Emplace");
        {
            myVector<Obj> v2;
            std::cout << "-- emplace_back(\"Alice\", 1) --\n";
            v2.emplace_back("Alice", 1);
            
            std::cout << "-- push_back(Obj(\"Bob\", 2)) --\n";
            v2.push_back(Obj("Bob", 2));

            std::cout << "-- reserve(10) (Should trigger moves) --\n";
            v2.reserve(10);
            
            std::cout << "v2 size: " << v2.size() << ", cap: " << v2.capacity() << "\n";
        } // v2 destructs here

        print_separator("Test 3: Copy & Move Vector");
        myVector<int> v3;
        v3.push_back(100);
        v3.push_back(200);

        std::cout << "-- Copy v4 = v3 --\n";
        myVector<int> v4 = v3;
        std::cout << "v4[0]: " << v4[0] << "\n";
        v4[0] = 999;
        std::cout << "v4 changed. v3[0]: " << v3[0] << " (Should be 100)\n";

        std::cout << "-- Move v5 = std::move(v3) --\n";
        myVector<int> v5 = std::move(v3);
        std::cout << "v5 size: " << v5.size() << "\n";
        std::cout << "v3 size: " << v3.size() << " (Should be 0)\n";

        print_separator("Test 4: Exception Safety (Mock)");
        std::cout << "Exception safety relies on logic verification mainly.\n";
        std::cout << "Current implementation uses std::move_if_noexcept.\n";

    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << "\n";
    }

    try {
        print_separator("Test 5: Insert (Modifiers)");
        myVector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(4);
        
        std::cout << "-- Initial: 1 2 4 --\n";
        
        std::cout << "-- insert(begin()+2, 3) --\n";
        auto it = v.insert(v.begin() + 2, 3);
        std::cout << "Ret val: " << *it << "\n";
        
        std::cout << "-- insert(begin(), 0) --\n";
        v.insert(v.begin(), 0);

        std::cout << "-- insert(end(), 5) --\n";
        v.insert(v.end(), 5);

        std::cout << "v: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(Expected: 0 1 2 3 4 5)\n";

        print_separator("Test 6: Erase (Modifiers)");
        // Current: 0 1 2 3 4 5
        std::cout << "-- erase(begin()) -> Remove 0 --\n";
        v.erase(v.begin());
        
        std::cout << "-- erase(end()-1) -> Remove 5 --\n";
        v.erase(v.end() - 1);

        std::cout << "-- erase(begin()+1, begin()+3) -> Remove 2, 3 --\n";
        // Current: 1 2 3 4
        // Index:   0 1 2 3
        v.erase(v.begin() + 1, v.begin() + 3);

        std::cout << "v: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(Expected: 1 4)\n";

        print_separator("Test 7: Resize");
        // Current v: 1 4 (size 2)
        std::cout << "-- resize(5, 10) -> Grow with value 10 --\n";
        v.resize(5, 10);
        std::cout << "v size: " << v.size() << ", cap: " << v.capacity() << "\n";
        std::cout << "v: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(Expected: 1 4 10 10 10)\n";

        std::cout << "-- resize(3) -> Shrink --\n";
        v.resize(3);
        std::cout << "v size: " << v.size() << "\n";
        std::cout << "v: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(Expected: 1 4 10)\n";

        std::cout << "-- resize(6) -> Grow with default Ctor (0) --\n";
        v.resize(6);
        std::cout << "v: ";
        for(size_t i=0; i<v.size(); ++i) std::cout << v[i] << " ";
        std::cout << "\n(Expected: 1 4 10 0 0 0)\n";

        print_separator("Test 8: Custom Allocator Check");
        // 使用自定义分配器声明 Vector
        myVector<int, DebugAllocator<int>> v_custom;
        std::cout << "Push 1, 2, 3 (expect alloc messages)\n";
        v_custom.push_back(1);
        v_custom.push_back(2);
        v_custom.push_back(3);

        std::cout << "Vector destruction (expect dealloc messages)\n";
        // main 结束时，v_custom 会析构，应该打印 deallocate 消息

    } catch (const std::exception& e) {
        std::cerr << "Modifiers Exception caught: " << e.what() << "\n";
    }

    return 0;
}
