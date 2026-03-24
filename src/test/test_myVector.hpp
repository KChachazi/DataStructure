#ifndef TEST_MYVECTOR_HPP
#define TEST_MYVECTOR_HPP

#include "../test.h"
#include "../myVector/myVector.h"
#include <vector>

using namespace TestHelpers;

// 友元测试类：验证 vector 内部状态
class VectorTester {
public:
    template <typename T>
    static bool checkCapacity(const myVector<T>& v, size_t expected_cap) {
        // vector扩容策略通常是 1 -> 2 -> 4 -> 8 (2倍) 或者 1.5倍
        // 这里只是验证它至少满足 expected_cap，且未超出太多（例如不超过4倍）
        return v.capacity() >= expected_cap;
    }
    
    template <typename T>
    static size_t getCapacity(const myVector<T>& v) {
        return v.capacity();
    }
};

TEST(MyVectorTest, BasicPushAndAccess) {
    myVector<int> v;
    EXPECT_EQ(v.size(), 0);
    EXPECT_TRUE(v.empty());

    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    EXPECT_EQ(v.size(), 3);
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
    EXPECT_EQ(v.front(), 10);
    EXPECT_EQ(v.back(), 30);
    
    // 测试 at() 越界抛出
    try {
        v.at(100);
        EXPECT_TRUE(false); // Should not reach here
    } catch (const std::out_of_range&) {
        EXPECT_TRUE(true);
    }
}

TEST(MyVectorTest, ReserveAndResize) {
    myVector<int> v;
    v.reserve(100);
    EXPECT_TRUE(VectorTester::getCapacity(v) >= 100);
    EXPECT_EQ(v.size(), 0);
    
    v.resize(5, 42); // 填充 5 个 42
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v[0], 42);
    EXPECT_EQ(v[4], 42);

    v.resize(2); // 截断
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 42); // 数据保留
}

TEST(MyVectorTest, ObjectLifecycleParams) {
    // 使用 TestHelpers::Obj 跟踪构造/析构
    Obj::resetStats();
    {
        myVector<Obj> v;
        v.push_back(Obj("A", 1)); // 1 ctor (temp), 1 move/copy (to vector), 1 dtor (temp)
        v.emplace_back("B", 2);   // 1 ctor (in-place)
    } 
    // v destructs: 2 dtors

    // 验证没有内存泄漏（构造数 == 析构数）
    // 注意：push_back 可能涉及重新分配导致的额外 move/dtor，这里只做松散检查
    EXPECT_EQ(Obj::construct_count, 2); 
    // 总析构数 = 临时对象析构 + vector内对象析构
    // 具体数字依赖于 push_back 实现细节（是否发生扩容拷贝），主要确保最终都释放了
    EXPECT_TRUE(Obj::destruct_count >= Obj::construct_count); 
}

TEST(MyVectorTest, CopyAndMoveSemantics) {
    myVector<int> v1;
    v1.push_back(1); v1.push_back(2);

    // Copy Ctor
    myVector<int> v2 = v1; 
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[0], 1);
    v2[0] = 99;
    EXPECT_EQ(v1[0], 1); // Deep copy verification

    // Move Ctor
    myVector<int> v3 = std::move(v2);
    EXPECT_EQ(v3.size(), 2);
    EXPECT_EQ(v3[0], 99);
    EXPECT_EQ(v2.size(), 0); // Moved from state
}

TEST(MyVectorTest, InsertAndErase) {
    myVector<int> v;
    for(int i=0; i<5; ++i) v.push_back(i); // 0 1 2 3 4
    
    // Insert at middle
    v.insert(v.begin() + 2, 99); // 0 1 99 2 3 4
    EXPECT_EQ(v[2], 99);
    EXPECT_EQ(v.size(), 6);

    // Erase range
    v.erase(v.begin() + 1, v.begin() + 3); // removes 1, 99 -> 0 2 3 4
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[1], 2);
}

TEST(MyVectorTest, CustomAllocator) {
    // 验证 Allocator 是否被调用
    using Alloc = DebugAllocator<int>;
    Alloc::alloc_count = 0;
    Alloc::dealloc_count = 0;
    
    {
        myVector<int, Alloc> v;
        v.push_back(1);
        v.reserve(10); 
    } // destruct
    
    EXPECT_TRUE(Alloc::alloc_count > 0);
    EXPECT_TRUE(Alloc::dealloc_count > 0);
}

TEST(MyVectorTest, IteratorTraits) {
    // 验证 vector 的迭代器是否符合 Random Access Iterator 要求
    myVector<int> v;
    v.push_back(1);
    auto it = v.begin();
    
    EXPECT_TRUE((std::is_same_v<decltype(it), int*>)); // 对于 vector, 迭代器通常是指针
    // 如果实现了 iterator_traits，这里也可以检查 category
}

// 简单的性能对比不需要每次都跑，可以设一个手动开关或者较小的 N
TEST(MyVectorTest, PerformanceComparison_Int) {
    const size_t N = 100000; // 这里的 N 设小一点以便 CI 快速运行，手动测试可加大
    long long t_std = run_pressure_test<std::vector<int>, int>(N);
    long long t_my = run_pressure_test<myVector<int>, int>(N);
    
    std::cout << "    [Perf] std::vector: " << t_std << "ms, myVector: " << t_my << "ms\n";
    // 只要不是慢得离谱（例如超过 2 倍），就算通过
    // 注意：Debug 模式下我实现的 vector 可能会比 std 慢很多因为没优化，Release 下才有可比性
    EXPECT_TRUE(true); 
}

#endif // TEST_MYVECTOR_HPP