#include <iostream>
#include <string>
#include "myVector/myVector.h"

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

    return 0;
}
