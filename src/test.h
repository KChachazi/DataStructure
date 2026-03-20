#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <ctime>

// ==========================================
// 简易测试框架 (MiniUnit)
// ==========================================

struct TestRegistry {
    struct TestEntry {
        std::string caseName;
        std::string testName;
        std::function<void()> func;
    };
    
    // 使用静态方法获取 vector，避免静态初始化顺序问题
    static std::vector<TestEntry>& getTests() {
        static std::vector<TestEntry> tests;
        return tests;
    }
    
    static bool currentTestFailed;
    
    static int registerTest(const char* caseName, const char* testName, std::function<void()> func) {
        getTests().push_back({caseName, testName, func});
        return 0;
    }
    
    static int runAllTests() {
        int passed = 0;
        int failed = 0;
        std::cout << "[==========] Running " << getTests().size() << " tests." << std::endl;
        
        for (const auto& test : getTests()) {
            currentTestFailed = false;
            std::cout << "[ RUN      ] " << test.caseName << "." << test.testName << std::endl;
            try {
                test.func();
            } catch (const std::exception& e) {
                std::cerr << "[  FAILED  ] Exception: " << e.what() << std::endl;
                currentTestFailed = true;
            } catch (...) {
                std::cerr << "[  FAILED  ] Unknown exception" << std::endl;
                currentTestFailed = true;
            }
            
            if (currentTestFailed) {
                std::cout << "[  FAILED  ] " << test.caseName << "." << test.testName << std::endl;
                failed++;
            } else {
                std::cout << "[       OK ] " << test.caseName << "." << test.testName << std::endl;
                passed++;
            }
        }
        
        std::cout << "[==========] Total: " << (passed + failed) 
                  << ", Passed: " << passed 
                  << ", Failed: " << failed << std::endl;
        
        return failed;
    }
};

inline bool TestRegistry::currentTestFailed = false;

// 测试宏
#define TEST(test_case_name, test_name) \
    void test_case_name##_##test_name(); \
    static int register_##test_case_name##_##test_name = TestRegistry::registerTest(#test_case_name, #test_name, test_case_name##_##test_name); \
    void test_case_name##_##test_name()

#define EXPECT_EQ(val1, val2) \
    if ((val1) != (val2)) { \
        std::cerr << "  " << __FILE__ << ":" << __LINE__ << ": Failure\n" \
                  << "  Expected: " << (val2) << "\n" \
                  << "  Actual:   " << (val1) << std::endl; \
        TestRegistry::currentTestFailed = true; \
    }

#define EXPECT_NE(val1, val2) \
    if ((val1) == (val2)) { \
        std::cerr << "  " << __FILE__ << ":" << __LINE__ << ": Failure\n" \
                  << "  Expected: " << (val1) << " != " << (val2) << std::endl; \
        TestRegistry::currentTestFailed = true; \
    }

#define EXPECT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "  " << __FILE__ << ":" << __LINE__ << ": Failure\n" \
                  << "  Expected true: " << #condition << std::endl; \
        TestRegistry::currentTestFailed = true; \
    }

#define EXPECT_FALSE(condition) \
    if ((condition)) { \
        std::cerr << "  " << __FILE__ << ":" << __LINE__ << ": Failure\n" \
                  << "  Expected false: " << #condition << std::endl; \
        TestRegistry::currentTestFailed = true; \
    }

// ==========================================
// 通用测试辅助工具 (Helpers)
// ==========================================

namespace TestHelpers {

    // 1. 生命周期追踪对象 (Lifecycle Tracker)
    struct Obj {
        std::string name;
        int id;
        static int construct_count;
        static int copy_count;
        static int move_count;
        static int destruct_count;

        static void resetStats() {
            construct_count = copy_count = move_count = destruct_count = 0;
        }

        Obj(std::string n, int i) : name(std::move(n)), id(i) {
            construct_count++;
            // std::cout << "  [Ctor] " << name << "\n";
        }
        
        Obj(const Obj& other) : name(other.name), id(other.id) {
            copy_count++;
            // std::cout << "  [Copy] " << name << "\n";
        }

        Obj(Obj&& other) noexcept : name(std::move(other.name)), id(other.id) {
            move_count++;
            other.name = "(moved)";
            // std::cout << "  [Move] " << name << "\n";
        }

        Obj& operator=(const Obj& other) {
            if (this != &other) {
                name = other.name;
                id = other.id;
                copy_count++;
            }
            return *this;
        }

        Obj& operator=(Obj&& other) noexcept {
            if (this != &other) {
                name = std::move(other.name);
                id = other.id;
                other.name = "(moved)";
                move_count++;
            }
            return *this;
        }

        ~Obj() {
            destruct_count++;
            // if (name != "(moved)") std::cout << "  [Dtor] " << name << "\n";
        }
        
        bool operator==(const Obj& other) const { return id == other.id && name == other.name; }
    };

    inline int Obj::construct_count = 0;
    inline int Obj::copy_count = 0;
    inline int Obj::move_count = 0;
    inline int Obj::destruct_count = 0;

    // 2. 自定义分配器 (DebugAllocator)
    template <typename T>
    struct DebugAllocator {
        using value_type = T;
        static int alloc_count;
        static int dealloc_count;

        DebugAllocator() = default;
        template <typename U> DebugAllocator(const DebugAllocator<U>&) {}

        T* allocate(std::size_t n) {
            alloc_count++;
            // std::cout << "[Alloc] " << n << " elements\n";
            return std::allocator<T>{}.allocate(n);
        }

        void deallocate(T* p, std::size_t n) {
            dealloc_count++;
            // std::cout << "[Dealloc] " << n << " elements\n";
            std::allocator<T>{}.deallocate(p, n);
        }
    };

    template <typename T> int DebugAllocator<T>::alloc_count = 0;
    template <typename T> int DebugAllocator<T>::dealloc_count = 0;

    template <typename T, typename U>
    bool operator==(const DebugAllocator<T>&, const DebugAllocator<U>&) { return true; }
    template <typename T, typename U>
    bool operator!=(const DebugAllocator<T>&, const DebugAllocator<U>&) { return false; }

    // 3. 大型POD类型 (HeavyPOD) - 用来做 memcpy 优化测试
    struct HeavyPOD {
        long data[8]; // 64 bytes
        HeavyPOD() = default;
        HeavyPOD(int v) { for(int i=0; i<8; ++i) data[i] = v; }
    };
    
    // 4. 压力测试辅助函数
    template <typename VecType, typename T>
    long long run_pressure_test(size_t N) {
        auto start = std::chrono::high_resolution_clock::now();
        VecType v;
        for (size_t i = 0; i < N; ++i) {
            v.push_back(static_cast<T>(i)); // 简单转换
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }
}

#endif // TEST_H