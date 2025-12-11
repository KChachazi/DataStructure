#ifndef MyINTVECTOR_H
#define MyINTVECTOR_H

#include <cstddef>
#include <stdexcept>

class intVector {
private:
    int *_data;
    size_t _size;
    size_t _capacity;
public:
    // Constructor, Destructor, copy constructor, and assignment operator
    intVector();
    ~intVector();
    explicit intVector(size_t size);
    intVector(const intVector& other);
    intVector& operator=(const intVector& other);
    // Move constructor and move assignment operator
    intVector(intVector&& other) noexcept;
    intVector& operator=(intVector&& other) noexcept;

    // stute functions
    size_t size() const;
    size_t capacity() const;
    bool empty() const;

    // Element access
    int& operator[] (size_t index);
    const int& operator[] (size_t index) const;
    int& at(size_t index);
    const int& at(size_t index) const;

    int& front();
    const int& front() const;
    int& back();
    const int& back() const;

    // modifier functions
    void push_back(int value);
    void pop_back();
    void clear();
    void reserve(size_t newCapacity);
    void resize(size_t newSize);

    // iterators
    using iterator = int*;
    using const_iterator = const int*;
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

private:
    void reallocate(size_t newCapacity);
};

#endif //MyINTVECTOR_H