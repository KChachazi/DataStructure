#ifndef TEST_MYLIST_HPP
#define TEST_MYLIST_HPP
#include "../test.h"
#include "../myList/myList.h"

class ListTester {
public:
    static void verify(const myList<int>& list) {
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
};

TEST(MyListTest, PushPopEraseCombination) {
    myList<int> list;
    try {
        list.pop_front(); // Should not throw
        list.pop_back();  // Should not throw
    } catch (...) {
        EXPECT_TRUE(false); // Should not reach here
    }
    
    list.push_back(100), list.push_back(200), list.push_back(300);
    list.erase(list.begin()); // Remove 100
    list.erase(++ list.begin()); // Remove 300
    // Now only 200 remains
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*list.begin(), 200);

    EXPECT_FALSE(list.empty());
    list.erase(list.begin()); // Remove 200
    EXPECT_TRUE(list.empty());
}

TEST(MyListTest, RangeForLoop) {
    myList<int> list;
    list.push_back(10);
    list.push_back(20);
    list.push_back(30);
    
    std::vector<int> actual;
    for (int val : list) {
        actual.push_back(val);
    }
    
    EXPECT_EQ(actual.size(), 3);
    EXPECT_EQ(actual[0], 10);
    EXPECT_EQ(actual[1], 20);
    EXPECT_EQ(actual[2], 30);
}

TEST(MyListTest, StringBasic) {
    myList<std::string> list;
    list.push_back("Hello");
    list.push_back("World");
    
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(*list.begin(), "Hello");
    EXPECT_EQ(*(++ list.begin()), "World");

    list.push_front("First");
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(*list.begin(), "First");

    list.pop_back();
    list.pop_front();
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*list.begin(), "Hello");

    list.erase(list.begin());
    EXPECT_TRUE(list.empty());
}

TEST(MyListTest, ClearFunction) {
    myList<int> list;
    for (int i = 0; i < 100; ++i) {
        list.push_back(i);
    }
    EXPECT_EQ(list.size(), 100);
    list.clear();
    EXPECT_TRUE(list.empty());

    for (int i = 0; i < 50; ++i) {
        list.push_back(i);
    }
    EXPECT_EQ(list.size(), 50);
    list.clear();
    EXPECT_TRUE(list.empty());
}

TEST(MyListTest, CopyConstructorAndAssignment) {
    myList<int> list1;
    for (int i = 0; i < 5; ++i) {
        list1.push_back(i);
    }
    EXPECT_EQ(list1.size(), 5);
    myList<int> list2(list1); // Copy constructor
    EXPECT_EQ(list2.size(), 5);
    EXPECT_EQ(*list2.begin(), 0);
    EXPECT_EQ(*(++ list2.begin()), 1);
    EXPECT_EQ(*(++ ++ list2.begin()), 2);
    EXPECT_EQ(*(++ ++ ++ list2.begin()), 3);
    EXPECT_EQ(*(++ ++ ++ ++ list2.begin()), 4);

    myList<int> list3;
    list3 = list1; // Copy assignment
    EXPECT_EQ(list3.size(), 5);
    EXPECT_EQ(*list3.begin(), 0);
    EXPECT_EQ(*(++ list3.begin()), 1);
    EXPECT_EQ(*(++ ++ list3.begin()), 2);
    EXPECT_EQ(*(++ ++ ++ list3.begin()), 3);
    EXPECT_EQ(*(++ ++ ++ ++ list3.begin()), 4);

    list1.clear();
    EXPECT_TRUE(list1.empty());
    EXPECT_EQ(list2.size(), 5); // list2 should remain unchanged
    EXPECT_EQ(list3.size(), 5); // list3 should remain unchanged
}

TEST(MyListTest, MoveConstructorAndAssignment) {
    myList<int> list1;
    for (int i = 0; i < 5; ++i) {
        list1.push_back(i);
    }
    myList<int> list2(std::move(list1)); // Move constructor
    EXPECT_TRUE(list1.empty());
    EXPECT_EQ(list2.size(), 5);
    EXPECT_EQ(*list2.begin(), 0);
    EXPECT_EQ(*(++ list2.begin()), 1);
    EXPECT_EQ(*(++ ++ list2.begin()), 2);
    EXPECT_EQ(*(++ ++ ++ list2.begin()), 3);
    EXPECT_EQ(*(++ ++ ++ ++ list2.begin()), 4);

    myList<int> list3;
    list3 = std::move(list2); // Move assignment
    EXPECT_TRUE(list2.empty());
    EXPECT_EQ(list3.size(), 5);
    EXPECT_EQ(*list3.begin(), 0);
    EXPECT_EQ(*(++ list3.begin()), 1);
    EXPECT_EQ(*(++ ++ list3.begin()), 2);
    EXPECT_EQ(*(++ ++ ++ list3.begin()), 3);
    EXPECT_EQ(*(++ ++ ++ ++ list3.begin()), 4);
}

#endif // TEST_MYLIST_HPP