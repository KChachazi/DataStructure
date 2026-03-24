#ifndef MYINTLIST_H
#define MYINTLIST_H

#include <iostream>

class intListNode {
public:
    int data;
    intListNode *prev, *next;
    intListNode(int value) : data(value), prev(nullptr), next(nullptr) {}
};

class intList_iterator {
public:
    intListNode *current;
    intList_iterator(intListNode *node) : current(node) {}
    int& operator*() { return current->data; }
    intList_iterator& operator++() { current = current->next; return *this; }
    intList_iterator operator++(int) { intList_iterator temp = *this; current = current->next; return temp; }
    intList_iterator& operator--() { current = current->prev; return *this; }
    intList_iterator operator--(int) { intList_iterator temp = *this; current = current->prev; return temp; }
    bool operator==(const intList_iterator &other) const { return current == other.current; }
    bool operator!=(const intList_iterator &other) const { return current != other.current; }
};

class intList_const_iterator {
public:
    const intListNode *current;
    intList_const_iterator(const intListNode *node) : current(node) {}
    const int& operator*() const { return current->data; }
    intList_const_iterator& operator++() { current = current->next; return *this; }
    intList_const_iterator operator++(int) { intList_const_iterator temp = *this; current = current->next; return temp; }
    intList_const_iterator& operator--() { current = current->prev; return *this; }
    intList_const_iterator operator--(int) { intList_const_iterator temp = *this; current = current->prev; return temp; }
    bool operator==(const intList_const_iterator &other) const { return current == other.current; }
    bool operator!=(const intList_const_iterator &other) const { return current != other.current; }
};

class intList {
    intListNode *head;
    size_t count;
public:
    intList();
    ~intList();
    // 迭代器类型定义
    using iterator = intList_iterator;
    using const_iterator = intList_const_iterator;
    // 数据操作
    void push_front(int value);
    void push_back(int value);
    void pop_front();
    void pop_back();
    void insert(iterator position, int value);
    void erase(iterator position);
    // 查询
    size_t size() const;
    // 迭代器
    iterator begin() { return iterator(head->next); }
    iterator end() { return iterator(head); }
    const_iterator begin() const { return const_iterator(head->next); }
    const_iterator end() const { return const_iterator(head); }
    
    // 友元测试类声明
    friend class IntListTester; // 允许访问私有成员进行完整性验证
private:
    // Debug
    void debugPrint();
};

intList::intList() : count(0) {
    head = new intListNode(0); // 哨兵节点
    head->prev = head;
    head->next = head;
}

intList::~intList() {
    intListNode *current = head->next;
    while (current != head) {
        intListNode *temp = current;
        current = current->next;
        delete temp;
    }
    delete head;
}

// ------------ 数据操作 ------------
void intList::push_front(int value) {
    insert(intList_iterator(head->next), value);
}

void intList::push_back(int value) {
    insert(intList_iterator(head), value); // 在哨兵节点前插入
}

void intList::pop_front() {
    if (count > 0) {
        erase(intList_iterator(head->next));
    }
}

void intList::pop_back() {
    if (count > 0) {
        erase(intList_iterator(head->prev));
    }
}

void intList::insert(iterator position, int value) {
    intListNode *newNode = new intListNode(value);
    newNode->prev = position.current->prev;
    newNode->next = position.current;
    position.current->prev->next = newNode;
    position.current->prev = newNode;
    count ++;
}

void intList::erase(iterator position) {
    if (position.current == head) return;
    position.current->prev->next = position.current->next;
    position.current->next->prev = position.current->prev;
    delete position.current;
    count --;
}

// ------------ 查询 ------------
size_t intList::size() const {
    return count;
}

// Debug
void intList::debugPrint() {
    intListNode *current = head->next;
    while (current != head) {
        std::cout << current->data << " ";
        current = current->next;
    }
    std::cout << std::endl;
}

#endif // MYINTLIST_H