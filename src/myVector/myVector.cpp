#include "myVector.h"

template <typename T>
myVector<T>::myVector() : _data(nullptr), _size(0), _capacity(0) {}

template <typename T>
myVector<T>::~myVector() {
    destroy_range(0, _size);
    operator delete[](static_cast<void*>(_data));
}

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

// ----------------------------

template <typename T>
void myVector<T>::allocate(size_t n) {
    // allocate 仅适用于空 vector
    _data = static_cast<T*>(operator new[](n * sizeof(T)));
    _capacity = n;
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
void myVector<T>::destroy_at(size_t index) {
    _data[index].~T();
}

template <typename T>
void myVector<T>::destroy_range(size_t from, size_t to) {
    for (size_t i = from; i < to; ++i) {
        _data[i].~T();
    }
}

template <typename T>
void myVector<T>::reallocate(size_t newCapacity) {
    if (newCapacity < _size) {
        throw std::length_error("newCapacity < _size");
    }

    T* newData = static_cast<T*>(operator new[](newCapacity * sizeof(T)));

    for (size_t i = 0; i < _size; i ++) {
        new(&newData[i]) T(std::move(_data[i]));
        _data[i].~T();
    }
    operator delete[](static_cast<void*>(_data));
    _data = newData;
    _capacity = newCapacity;
}

// ----------------------------

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

// ----------------------------

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

// ----------------------------

template <typename ... Args>
void myVector<T>::emplace_back(Args&& ... args) {
    if (_size >= _capacity) {
        size_t newCapacity = (_capacity == 0) > 1 : _capacity * 2;
        reallocate(newCapacity);
    }
    new(&_data[_size]) T(std::forward<Args>(args)...);
    _size ++;
}