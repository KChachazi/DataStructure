#ifndef MY_VECTOR_H
#define MY_VECTOR_H

#include <cstddef>      // size_t
#include <utility>      // std::move
#include <new>          // placement new
#include <stdexcept>    // std::out_of_range
#include <memory>       // std::allocator, std::allocator_traits
#include <type_traits>  // std::is_trivially_copyable
#include <cstring>      // std::memcpy

template <typename T, typename Alloc = std::allocator<T>>
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
    Alloc allocator;
    using traits = std::allocator_traits<Alloc>;
    // [[no_unique_address]] Alloc allocator; // C++20 可用，节省空间
    void allocate(size_t n);
    void construct_at(size_t index);
    void construct_at(size_t index, const T& value);
    void construct_at(size_t index, T&& value);
    template <typename ... Args>
    void construct_at(size_t index, Args&& ... args);
    void reallocate(size_t newCapacity);
    void destroy_at(size_t index);
    void destroy_range(size_t from, size_t to);

public:
    void swap(myVector& other) noexcept;
};


// ==========================================================
// Implementation - Constructors / Destructor
// ==========================================================

template <typename T, typename Alloc>
myVector<T, Alloc>::myVector() : _data(nullptr), _size(0), _capacity(0) {}

template <typename T, typename Alloc>
myVector<T, Alloc>::~myVector() {
    destroy_range(0, _size);
    traits::deallocate(allocator, _data, _capacity);
}

template <typename T, typename Alloc>
myVector<T, Alloc>::myVector(const myVector& other)
    : _data(nullptr), _size(0), _capacity(0), allocator(traits::select_on_container_copy_construction(other.allocator)) {
        if (other._size > 0) {
            allocate(other._capacity);
            for (size_t i = 0; i < other._size; i ++) {
                construct_at(i, other._data[i]);
            }
        }
        _size = other._size;
    }

template <typename T, typename Alloc>
myVector<T, Alloc>& myVector<T, Alloc>::operator=(const myVector& other) {
    if (this != &other) {
        clear();
        if (other._size > _capacity) {
            traits::deallocate(allocator, _data, _capacity);
            allocate(other._capacity);
        }
        for (size_t i = 0; i < other._size; i ++) {
            construct_at(i, other._data[i]);
        }
    }
    _size = other._size;
    return *this;
}

template <typename T, typename Alloc>
myVector<T, Alloc>::myVector(myVector&& other) noexcept
    : _data(other._data), _size(other._size), _capacity(other._capacity), allocator(std::move(other.allocator)) {
        other._data = nullptr;
        other._size = 0;
        other._capacity = 0;
    }

template <typename T, typename Alloc>
myVector<T, Alloc>& myVector<T, Alloc>::operator=(myVector&& other) noexcept {
    if (this != &other) {
        clear();
        traits::deallocate(allocator, _data, _capacity);
        _data = other._data;
        _size = other._size;
        _capacity = other._capacity;
        allocator = std::move(other.allocator);
        other._data = nullptr;
        other._size = 0;
        other._capacity = 0;
    }
    return *this;
}

// ==========================================================
// Implementation - Capacity
// ==========================================================

template <typename T, typename Alloc>
size_t myVector<T, Alloc>::size() const noexcept {
    return _size;
}

template <typename T, typename Alloc>
size_t myVector<T, Alloc>::capacity() const noexcept {
    return _capacity;
}

template <typename T, typename Alloc>
bool myVector<T, Alloc>::empty() const noexcept {
    return _size == 0;
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::resize(size_t newSize) {
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

template <typename T, typename Alloc>
void myVector<T, Alloc>::resize(size_t newSize, const T& value) {
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

template <typename T, typename Alloc>
T& myVector<T, Alloc>::operator[](size_t index) {
    return _data[index];
}

template <typename T, typename Alloc>
const T& myVector<T, Alloc>::operator[](size_t index) const {
    return _data[index];
}

template <typename T, typename Alloc>
T& myVector<T, Alloc>::at(size_t index) {
    if (index >= _size) {
        throw std::out_of_range("Index out of range");
    }
    return _data[index];
}

template <typename T, typename Alloc>
const T& myVector<T, Alloc>::at(size_t index) const {
    if (index >= _size) {
        throw std::out_of_range("Index out of range");
    }
    return _data[index];
}

template <typename T, typename Alloc>
T& myVector<T, Alloc>::front() {
    return _data[0];
}

template <typename T, typename Alloc>
const T& myVector<T, Alloc>::front() const {
    return _data[0];
}

template <typename T, typename Alloc>
T& myVector<T, Alloc>::back() {
    return _data[_size - 1];
}

template <typename T, typename Alloc>
const T& myVector<T, Alloc>::back() const {
    return _data[_size - 1];
}

// ==========================================================
// Implementation - Iterators
// ==========================================================

template <typename T, typename Alloc>
typename myVector<T, Alloc>::iterator myVector<T, Alloc>::begin() noexcept {
    return _data;
}

template <typename T, typename Alloc>
typename myVector<T, Alloc>::iterator myVector<T, Alloc>::end() noexcept {
    return _data + _size;
}

template <typename T, typename Alloc>
typename myVector<T, Alloc>::const_iterator myVector<T, Alloc>::begin() const noexcept {
    return _data;
}

template <typename T, typename Alloc>
typename myVector<T, Alloc>::const_iterator myVector<T, Alloc>::end() const noexcept {
    return _data + _size;
}

// ==========================================================
// Implementation - Modifiers
// ==========================================================

template <typename T, typename Alloc>
void myVector<T, Alloc>::push_back(const T& value) {
    if (_size >= _capacity) {
        size_t newCapacity = (_capacity == 0) ? 1 : _capacity * 2;
        reallocate(newCapacity);
    }
    construct_at(_size, value);
    _size ++;
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::pop_back() {
    if (_size > 0) {
        -- _size;
        destroy_at(_size);
    }
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::clear() {
    destroy_range(0, _size);
    _size = 0;
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::reserve(size_t newCapacity) {
    if (newCapacity <= _capacity) {
        return;
    }
    reallocate(newCapacity);
}

template <typename T, typename Alloc>
template <typename ... Args>
void myVector<T, Alloc>::emplace_back(Args&& ... args) {
    if (_size >= _capacity) {
        size_t newCapacity = (_capacity == 0) ? 1 : _capacity * 2;
        reallocate(newCapacity);
    }
    construct_at(_size, std::forward<Args>(args)...);
    _size ++;
}

template <typename T, typename Alloc>
typename myVector<T, Alloc>::iterator myVector<T, Alloc>::insert(const_iterator pos, const T& value) {
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

template <typename T, typename Alloc>
typename myVector<T, Alloc>::iterator myVector<T, Alloc>::insert(const_iterator pos, T&& value) {
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

template <typename T, typename Alloc>
typename myVector<T, Alloc>::iterator myVector<T, Alloc>::erase(const_iterator pos) {
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

template <typename T, typename Alloc>
typename myVector<T, Alloc>::iterator myVector<T, Alloc>::erase(const_iterator first, const_iterator last) {
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

template <typename T, typename Alloc>
void myVector<T, Alloc>::allocate(size_t n) {
    // allocate 仅适用于空 vector
    _data = traits::allocate(allocator, n);
    _capacity = n;
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::construct_at(size_t index) {
    traits::construct(allocator, &_data[index]);
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::construct_at(size_t index, const T& value) {
    traits::construct(allocator, &_data[index], value);
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::construct_at(size_t index, T&& value) {
    traits::construct(allocator, &_data[index], std::move(value));
}

template <typename T, typename Alloc>
template <typename ... Args>
void myVector<T, Alloc>::construct_at(size_t index, Args&& ... args) {
    traits::construct(allocator, &_data[index], std::forward<Args>(args)...);
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::reallocate(size_t newCapacity) {
    // 1. 分配新内存
    T* newData = traits::allocate(allocator, newCapacity); 
    
    // 2. 尝试在新内存上构造元素
    // 性能优化
    // 编译器分支(C++17)：如果 T 是 trivially copyable 的，则直接 memcpy；否则逐个 move 构造
    if constexpr (std::is_trivially_copyable_v<T>) {
        std::memcpy(newData, _data, _size * sizeof(T));
    } else {
        size_t i = 0;
        try {
            for (; i < _size; i ++) {
                traits::construct(allocator, &newData[i], std::move_if_noexcept(_data[i]));
            }
        } catch (...) {
            // 回滚: 销毁已构造的新元素，释放新内存
            for (size_t j = 0; j < i; j ++) {
                traits::destroy(allocator, &newData[j]);
            }
            traits::deallocate(allocator, newData, newCapacity);
            throw; // 重新抛出异常，保持 Strong Guarantee
        }
    }

    // 3. 释放旧资源
    for (size_t k = 0; k < _size; k ++) {
        traits::destroy(allocator, &_data[k]);
    }
    traits::deallocate(allocator, _data, _capacity);

    // 4. 更新指针
    _data = newData;
    _capacity = newCapacity;
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::swap(myVector& other) noexcept {
    using std::swap;
    swap(_data, other._data);
    swap(_size, other._size);
    swap(_capacity, other._capacity);
    swap(allocator, other.allocator);
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::destroy_at(size_t index) {
    traits::destroy(allocator, &_data[index]);
}

template <typename T, typename Alloc>
void myVector<T, Alloc>::destroy_range(size_t from, size_t to) {
    for (size_t i = from; i < to; i ++) {
        destroy_at(i);
    }
}

#endif // MY_VECTOR_H
