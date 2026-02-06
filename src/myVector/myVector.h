#ifndef MY_VECTOR_H
#define MY_VECTOR_H

#include <cstddef>      // size_t
#include <utility>      // std::move
#include <new>          // placement new
#include <stdexcept>    // std::out_of_range

template <typename T>
class myVector {
private:
    T*      _data;      // 指向原始内存（不是“数组”）
    size_t  _size;      // 已构造元素个数
    size_t  _capacity;  // 可容纳元素个数（内存块大小）

public:
    /* ===== 构造 / 析构 ===== */
    myVector();
    ~myVector();
    myVector(const myVector& other);
    myVector& operator=(const myVector& other);
    myVector(myVector&& other) noexcept;
    myVector& operator=(myVector&& other) noexcept;

    /* ===== 容量相关 ===== */
    size_t size() const noexcept;
    size_t capacity() const noexcept;
    bool empty() const noexcept;
    void resize(size_t newSize);
    void resize(size_t newSize, const T& value);

    /* ===== 元素访问 ===== */
    T& operator[](size_t index);
    const T& operator[](size_t index) const;
    T& at(size_t index);
    const T& at(size_t index) const;
    T& front();
    const T& front() const;
    T& back();
    const T& back() const;

    /* ===== 迭代器 ===== */
    using iterator = T*;
    using const_iterator = const T*;
    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;

    /* ===== 修改器 ===== */
    void push_back(const T& value);
    void pop_back();
    void clear();
    void reserve(size_t newCapacity);
    template <typename ... Args>
    void emplace_back(Args&& ... args);
    iterator insert(const_iterator pos, const T& value);
    iterator insert(const_iterator pos, T&& value);
    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);

private:
    /* ===== 内部工具 ===== */
    void allocate(size_t n);
    void construct_at(size_t index);
    void construct_at(size_t index, const T& value);
    void construct_at(size_t index, T&& value);
    template <typename ... Args>
    void construct_at(size_t index, Args&& ... args);
    void reallocate(size_t newCapacity);
    void destroy_at(size_t index);
    void destroy_range(size_t from, size_t to);
};


// ==========================================================
// Implementation - Constructors / Destructor
// ==========================================================

template <typename T>
myVector<T>::myVector() : _data(nullptr), _size(0), _capacity(0) {}

template <typename T>
myVector<T>::~myVector() {
    destroy_range(0, _size);
    operator delete[](static_cast<void*>(_data));
}

template <typename T>
myVector<T>::myVector(const myVector& other)
    : _data(nullptr), _size(0), _capacity(0) {
        if (other._size > 0) {
            allocate(other._capacity);
            for (size_t i = 0; i < other._size; i ++) {
                construct_at(i, other._data[i]);
            }
        }
        _size = other._size;
    }

template <typename T>
myVector<T>& myVector<T>::operator=(const myVector& other) {
    if (this != &other) {
        clear();
        if (other._size > _capacity) {
            operator delete[](static_cast<void*>(_data));
            allocate(other._capacity);
        }
        for (size_t i = 0; i < other._size; i ++) {
            construct_at(i, other._data[i]);
        }
    }
    _size = other._size;
    return *this;
}

template <typename T>
myVector<T>::myVector(myVector&& other) noexcept
    : _data(other._data), _size(other._size), _capacity(other._capacity) {
        other._data = nullptr;
        other._size = 0;
        other._capacity = 0;
    }

template <typename T>
myVector<T>& myVector<T>::operator=(myVector&& other) noexcept {
    if (this != &other) {
        clear();
        operator delete[](static_cast<void*>(_data));
        _data = other._data;
        _size = other._size;
        _capacity = other._capacity;
        other._data = nullptr;
        other._size = 0;
        other._capacity = 0;
    }
    return *this;
}

// ==========================================================
// Implementation - Capacity
// ==========================================================

template <typename T>
size_t myVector<T>::size() const noexcept {
    return _size;
}

template <typename T>
size_t myVector<T>::capacity() const noexcept {
    return _capacity;
}

template <typename T>
bool myVector<T>::empty() const noexcept {
    return _size == 0;
}

template <typename T>
void myVector<T>::resize(size_t newSize) {
    if (newSize < _size) {
        destroy_range(newSize, _size);
    }
    else if (newSize > _size) {
        if (newSize > _capacity) {
            size_t newCapacity = (_capacity == 0) ? 1 : _capacity;
            while (newCapacity < newSize) {
                newCapacity *= 2;
            }
            reallocate(newCapacity);
        }
        for (size_t i = _size; i < newSize; i ++) {
            construct_at(i);
        }
    }
    _size = newSize;
}

template <typename T>
void myVector<T>::resize(size_t newSize, const T& value) {
    if (newSize < _size) {
        destroy_range(newSize, _size);
    }
    else if (newSize > _size) {
        if (newSize > _capacity) {
            size_t newCapacity = (_capacity == 0) ? 1 : _capacity;
            while (newCapacity < newSize) {
                newCapacity *= 2;
            }
            reallocate(newCapacity);
        }
        for (size_t i = _size; i < newSize; i ++) {
            construct_at(i, value);
        }
    }
    _size = newSize;
}

// ==========================================================
// Implementation - Element Access
// ==========================================================

template <typename T>
T& myVector<T>::operator[](size_t index) {
    return _data[index];
}

template <typename T>
const T& myVector<T>::operator[](size_t index) const {
    return _data[index];
}

template <typename T>
T& myVector<T>::at(size_t index) {
    if (index >= _size) {
        throw std::out_of_range("Index out of range");
    }
    return _data[index];
}

template <typename T>
const T& myVector<T>::at(size_t index) const {
    if (index >= _size) {
        throw std::out_of_range("Index out of range");
    }
    return _data[index];
}

template <typename T>
T& myVector<T>::front() {
    return _data[0];
}

template <typename T>
const T& myVector<T>::front() const {
    return _data[0];
}

template <typename T>
T& myVector<T>::back() {
    return _data[_size - 1];
}

template <typename T>
const T& myVector<T>::back() const {
    return _data[_size - 1];
}

// ==========================================================
// Implementation - Iterators
// ==========================================================

template <typename T>
typename myVector<T>::iterator myVector<T>::begin() noexcept {
    return _data;
}

template <typename T>
typename myVector<T>::iterator myVector<T>::end() noexcept {
    return _data + _size;
}

template <typename T>
typename myVector<T>::const_iterator myVector<T>::begin() const noexcept {
    return _data;
}

template <typename T>
typename myVector<T>::const_iterator myVector<T>::end() const noexcept {
    return _data + _size;
}

// ==========================================================
// Implementation - Modifiers
// ==========================================================

template <typename T>
void myVector<T>::push_back(const T& value) {
    if (_size >= _capacity) {
        size_t newCapacity = (_capacity == 0) ? 1 : _capacity * 2;
        reallocate(newCapacity);
    }
    construct_at(_size, value);
    _size ++;
}

template <typename T>
void myVector<T>::pop_back() {
    if (_size > 0) {
        -- _size;
        destroy_at(_size);
    }
}

template <typename T>
void myVector<T>::clear() {
    destroy_range(0, _size);
    _size = 0;
}

template <typename T>
void myVector<T>::reserve(size_t newCapacity) {
    if (newCapacity <= _capacity) {
        return;
    }
    reallocate(newCapacity);
}

template <typename T>
template <typename ... Args>
void myVector<T>::emplace_back(Args&& ... args) {
    if (_size >= _capacity) {
        size_t newCapacity = (_capacity == 0) ? 1 : _capacity * 2;
        reallocate(newCapacity);
    }
    construct_at(_size, std::forward<Args>(args)...);
    _size ++;
}

template <typename T>
typename myVector<T>::iterator myVector<T>::insert(const_iterator pos, const T& value) {
    if (pos < begin() || pos > end()) {
        throw std::out_of_range("Insert position out of range");
    }
    size_t index = pos - begin();
    // 当心迭代器失效陷阱 若先 reallocate 则 pos 失效
    // 所以必须先计算 index
    if (_size >= _capacity) {
        size_t newCapacity = (_capacity == 0) ? 1 : _capacity * 2;
        reallocate(newCapacity);
    }
    for (size_t i = _size; i > index; i --) {
        construct_at(i, std::move(_data[i - 1]));
        destroy_at(i - 1);
    }
    construct_at(index, value);
    _size ++;
    return begin() + index;
}

template <typename T>
typename myVector<T>::iterator myVector<T>::insert(const_iterator pos, T&& value) {
    if (pos < begin() || pos > end()) {
        throw std::out_of_range("Insert position out of range");
    }
    size_t index = pos - begin();
    if (_size >= _capacity) {
        size_t newCapacity = (_capacity == 0) ? 1 : _capacity * 2;
        reallocate(newCapacity);
    }
    for (size_t i = _size; i > index; i --) {
        construct_at(i, std::move(_data[i - 1]));
        destroy_at(i - 1);
    }
    construct_at(index, std::move(value));
    _size ++;
    return begin() + index;
}

template <typename T>
typename myVector<T>::iterator myVector<T>::erase(const_iterator pos) {
    if (pos < begin() || pos >= end()) {
        throw std::out_of_range("Erase position out of range");
    }
    size_t index = pos - begin();
    for (size_t i = index; i < _size; i ++) {
        if (i + 1 < _size) {
            _data[i] = std::move(_data[i + 1]);
        }
    }
    _size --;
    destroy_at(_size);
    return begin() + index;
}

template <typename T>
typename myVector<T>::iterator myVector<T>::erase(const_iterator first, const_iterator last) {
    if (first < begin() || last > end() || first > last) {
        throw std::out_of_range("Erase range out of range");
    }
    size_t startIndex = first - begin(), endIndex = last - begin();
    for (size_t i = startIndex; i < _size; i ++) {
        if (i + (endIndex - startIndex) < _size) {
            _data[i] = std::move(_data[i + (endIndex - startIndex)]);
        }
    }
    size_t numToDestroy = endIndex - startIndex;
    for (size_t i = 0; i < numToDestroy; i ++) {
        _size --;
        destroy_at(_size);
    }
    return begin() + startIndex;
}

// ==========================================================
// Implementation - Internal Tools
// ==========================================================

template <typename T>
void myVector<T>::allocate(size_t n) {
    // allocate 仅适用于空 vector
    _data = static_cast<T*>(operator new[](n * sizeof(T)));
    _capacity = n;
}

template <typename T>
void myVector<T>::construct_at(size_t index) {
    new(&_data[index]) T();
}

template <typename T>
void myVector<T>::construct_at(size_t index, const T& value) {
    new(&_data[index]) T(value);
}

template <typename T>
void myVector<T>::construct_at(size_t index, T&& value) {
    new(&_data[index]) T(std::move(value));
}

template <typename T>
template <typename ... Args>
void myVector<T>::construct_at(size_t index, Args&& ... args) {
    new(&_data[index]) T(std::forward<Args>(args)...);
}

template <typename T>
void myVector<T>::reallocate(size_t newCapacity) {
    if (newCapacity < _size) {
        throw std::length_error("newCapacity < _size");
    }

    T* newData = static_cast<T*>(operator new[](newCapacity * sizeof(T)));
    size_t i = 0;
    try {
        for (; i < _size; i ++) {
            new(&newData[i]) T(std::move_if_noexcept(_data[i]));
        }
    } catch (...) {
        for (size_t j = 0; j < i; j ++) {
            newData[j].~T();
        }
        operator delete[](static_cast<void*>(newData));
        throw;
    }

    for (size_t k = 0; k < _size; k ++) {
        _data[k].~T();
    }
    operator delete[](static_cast<void*>(_data));

    _data = newData;
    _capacity = newCapacity;
}

template <typename T>
void myVector<T>::destroy_at(size_t index) {
    _data[index].~T();
}

template <typename T>
void myVector<T>::destroy_range(size_t from, size_t to) {
    for (size_t i = from; i < to; i ++) {
        _data[i].~T();
    }
}

#endif // MY_VECTOR_H
