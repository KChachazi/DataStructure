#ifndef TEST_INTLIST_HPP
#define TEST_INTLIST_HPP

#include "../test.h"
#include "../myList/intList/intList.h"

// 友元测试类 - 验证内部结构完整性 (白盒)
class IntListTester {
public:
    static void verify(const intList& list) {
        auto* head = list.head; // Should be accessible if friend
        auto* current = head->next;
        size_t count = 0;
        
        // Check forward links
        while (current != head) {
            EXPECT_EQ(current->prev->next, current);
            EXPECT_EQ(current->next->prev, current);
            current = current->next;
            count++;
            if (count > list.size() + 10) {
                EXPECT_TRUE(false); // Infinite loop
                break;
            }
        }
        // Check sentinel
        EXPECT_EQ(head->prev->next, head);
        EXPECT_EQ(head->next->prev, head);
        EXPECT_EQ(list.size(), count);
    }
    
    // 辅助函数：根据索引获取值（O(N)），用于验证顺序
    static int get_at(intList& list, size_t index) {
        size_t i = 0;
        for (int val : list) {
            if (i == index) return val;
            i++;
        }
        return -1; // Error
    }
};

TEST(IntListTest, BasicPushAndSize) {
    intList list;
    EXPECT_EQ(list.size(), 0);
    IntListTester::verify(list);

    list.push_back(10);
    EXPECT_EQ(list.size(), 1);
    IntListTester::verify(list);

    list.push_front(5); // 5, 10
    EXPECT_EQ(list.size(), 2);
    IntListTester::verify(list);

    auto it = list.begin();
    EXPECT_EQ(*it, 5);
    ++it;
    EXPECT_EQ(*it, 10);
}

TEST(IntListTest, PopBackFront) {
    intList list;
    list.push_back(1); 
    list.push_back(2);
    list.push_back(3);
    // 1 2 3
    
    list.pop_front(); // 2 3
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(*list.begin(), 2);
    IntListTester::verify(list);

    list.pop_back(); // 2
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*list.begin(), 2);
    IntListTester::verify(list);
    
    list.pop_back(); // empty
    EXPECT_EQ(list.size(), 0);
    IntListTester::verify(list);
}

TEST(IntListTest, InsertErase) {
    intList list;
    list.push_back(10);
    list.push_back(30);
    // 10 30
    
    auto it = list.begin(); 
    ++it; // at 30
    
    list.insert(it, 20); // 10 20 30
    EXPECT_EQ(list.size(), 3);
    IntListTester::verify(list);
    
    // Verify sequence
    it = list.begin();
    EXPECT_EQ(*it, 10); ++it;
    EXPECT_EQ(*it, 20); ++it;
    EXPECT_EQ(*it, 30);
    
    // Erase 20
    it = list.begin(); ++it; // at 20
    list.erase(it); // 10 30
    EXPECT_EQ(list.size(), 2);
    IntListTester::verify(list);
    
    it = list.begin();
    EXPECT_EQ(*it, 10); ++it;
    EXPECT_EQ(*it, 30);
}

TEST(IntListTest, RangeForLoop) {
    intList list;
    list.push_back(100);
    list.push_back(200);
    list.push_back(300);
    
    std::vector<int> actual;
    for(auto val : list) {
        actual.push_back(val);
    }
    
    EXPECT_EQ(actual.size(), 3);
    EXPECT_EQ(actual[0], 100);
    EXPECT_EQ(actual[1], 200);
    EXPECT_EQ(actual[2], 300);
}

#endif // TEST_INTLIST_HPP