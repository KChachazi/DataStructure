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

    /* ===== 元素访问 ===== */
    T& operator[](size_t index);
    const T& operator[](size_t index) const;

    T& at(size_t index);
    const T& at(size_t index) const;

    T& front();
    const T& front() const;

    T& back();
    const T& back() const;

    /* ===== 修改器 ===== */
    void push_back(const T& value);
    void pop_back();
    void clear();
    void reserve(size_t newCapacity);
    template <typename ... Args>
    void emplace_back(Args&& ... args);

    /* ===== 迭代器 ===== */
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;

private:
    /* ===== 内部工具 ===== */
    void allocate(size_t n);
    void construct_at(size_t index, const T& value);
    void construct_at(size_t index, T&& value);
    void reallocate(size_t newCapacity);
    void destroy_at(size_t index);
    void destroy_range(size_t from, size_t to);
};

#endif // MY_VECTOR_H
