#ifndef MYLIST_H
#define MYLIST_H

#include <iostream>
#include <cstddef>

template <typename T>
class ListNode {
public:
    T data;
    ListNode *prev, *next;
    ListNode() : prev(nullptr), next(nullptr) {}
    ListNode(const T& value) : data(value), prev(nullptr), next(nullptr) {}
};

template <typename T>
class myList_iterator {
public:
    ListNode<T> *current;
    myList_iterator(ListNode<T> *node) : current(node) {}
    T& operator*() { return current->data; }
    // 自增和自减
    myList_iterator& operator++() { current = current->next; return *this; }
    myList_iterator operator++(int) { myList_iterator temp = *this; current = current->next; return temp; }
    myList_iterator& operator--() { current = current->prev; return *this; }
    myList_iterator operator--(int) { myList_iterator temp = *this; current = current->prev; return temp; }
    // 比较运算符
    bool operator==(const myList_iterator &other) const { return current == other.current; }
    bool operator!=(const myList_iterator &other) const { return current != other.current; }
};

template <typename T>
class myList_const_iterator {
public:
    const ListNode<T> *current;
    myList_const_iterator(const ListNode<T> *node) : current(node) {}
    const T& operator*() const { return current->data; }
    // 自增和自减
    myList_const_iterator& operator++() { current = current->next; return *this; }
    myList_const_iterator operator++(int) { myList_const_iterator temp = *this; current = current->next; return temp; }
    myList_const_iterator& operator--() { current = current->prev; return *this; }
    myList_const_iterator operator--(int) { myList_const_iterator temp = *this; current = current->prev; return temp; }
    // 比较运算符
    bool operator==(const myList_const_iterator &other) const { return current == other.current; }
    bool operator!=(const myList_const_iterator &other) const { return current != other.current; }
};

template <typename T>
class myList {
    ListNode<T> *head;
    size_t _size;
public:
    myList();
    ~myList();
    using iterator = myList_iterator<T>;
    using const_iterator = myList_const_iterator<T>;
    // 数据操作
    void push_front(const T& value);
    void push_back(const T& value);
    void pop_front();
    void pop_back();
    void insert(iterator position, const T& value);
    void erase(iterator position);
    // 查询
    size_t size() const;
    bool empty() const;
    // 迭代器
    iterator begin() { return iterator(head->next); }
    iterator end() { return iterator(head); }
    const_iterator begin() const { return const_iterator(head->next); }
    const_iterator end() const { return const_iterator(head); }
    
private:
    // debug
    void debugPrint() const;
};

// ------------ 构造与析构 ------------
template <typename T>
myList<T>::myList() : _size(0) {
    head = new ListNode<T>(); // 哨兵节点
    head->next = head;
    head->prev = head;
}

template <typename T>
myList<T>::~myList() {
    ListNode<T> *current = head->next;
    while (current != head) {
        ListNode<T> *temp = current;
        current = current->next;
        delete temp;
    }
    delete head;
}

// --------------------- 数据操作 ---------------------
template <typename T>
void myList<T>::push_front(const T& value) {
    insert(iterator(head->next), value);
}

template <typename T>
void myList<T>::push_back(const T& value) {
    insert(iterator(head), value);
}

template <typename T>
void myList<T>::pop_front() {
    if (_size > 0) {
        erase(iterator(head->next));
    }
}

template <typename T>
void myList<T>::pop_back() {
    if (_size > 0) {
        erase(iterator(head->prev));
    }
}

template <typename T>
void myList<T>::insert(iterator position, const T& value) {
    ListNode<T> *newNode = new ListNode<T>(value);
    ListNode<T> *posNode = position.current;
    newNode->prev = posNode->prev;
    newNode->next = posNode;
    posNode->prev->next = newNode;
    posNode->prev = newNode;
    ++ _size;
}

template <typename T>
void myList<T>::erase(iterator position) {
    if (position.current == head) return ;
    position.current->next->prev = position.current->prev;
    position.current->prev->next = position.current->next;
    delete position.current;
    -- _size;
}

// --------------------- 查询 ---------------------
template <typename T>
size_t myList<T>::size() const {
    return _size;
}

// debug
template <typename T>
void myList<T>::debugPrint() const {
    ListNode<T> *current = head->next;
    while (current != head) {
        std::cout << current->data << " ";
        current = current->next;
    }
    std::cout << std::endl;
}


#endif // MYLIST_H