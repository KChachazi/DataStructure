#include "intVector.h"
#include <algorithm>
#include <cstring>

intVector::intVector() : _data(nullptr), _size(0), _capacity(0) {}

intVector::~intVector() {
    delete[] _data;
}

intVector::intVector(size_t size) : _data(new int[size]), _size(size), _capacity(size) {
    std::fill(_data, _data + _size, 0);
}

intVector::intVector(const intVector& other) : _data(new int[other._capacity]), _size(other._size), _capacity(other._capacity) {
    std::memcpy(_data, other._data, _size * sizeof(int));
}

intVector& intVector::operator=(const intVector& other) {
    intVector temp(other);
    swap(temp);
    return *this;
}

intVector::intVector(intVector&& other) noexcept : _data(other._data), _size(other._size), _capacity(other._capacity) {
    other._data = nullptr;
    other._size = 0;
    other._capacity = 0;
}

intVector& intVector::operator=(intVector&& other) noexcept {
    if (this != &other) {
        delete[] _data;
        _data = other._data;
        _size = other._size;
        _capacity = other._capacity;
        other._data = nullptr;
        other._size = 0;
        other._capacity = 0;
    }
    return *this;
}

size_t intVector::size() const {
    return _size;
}

size_t intVector::capacity() const {
    return _capacity;
}

bool intVector::empty() const {
    return _size == 0;
}

int& intVector::operator[](size_t index) {
    return _data[index];
}

const int& intVector::operator[](size_t index) const {
    return _data[index];
}

int& intVector::at(size_t index) {
    if (index >= _size) {
        throw std::out_of_range("Index out of range");
    }
    return _data[index];
}

const int& intVector::at(size_t index) const {
    if (index >= _size) {
        throw std::out_of_range("Index out of range");
    }
    return _data[index];
}

int& intVector::front() {
    return _data[0];
}

const int& intVector::front() const {
    return _data[0];
}

int& intVector::back() {
    return _data[_size - 1];
}

const int& intVector::back() const {
    return _data[_size - 1];
}

void intVector::pop_back() {
    if (_size > 0) {
        --_size;
    }
}

void intVector::reserve(size_t newCapacity) {
    if (newCapacity <= _capacity) {
        return;
    }
    reallocate(newCapacity);
}

void intVector::reallocate(size_t newCapacity) {
    if (newCapacity < _size) {
        throw std::length_error("newCapacity < _size");
    }

    int *newData = new int[newCapacity];

    std::memcpy(newData, _data, _size * sizeof(int));
    delete[] _data;
    _data = newData;
    _capacity = newCapacity;
}

void intVector::push_back(int value) {
    if (_size >= _capacity) {
        size_t newCapacity = (_capacity == 0) ? 1 : _capacity * 2;
        reallocate(newCapacity);
    }
    _data[_size++] = value;
}

void intVector::clear() {
    _size = 0;
}

int* intVector::begin() {
    return _data;
}

int* intVector::end() {
    return _data + _size;
}

const int* intVector::begin() const {
    return _data;
}

const int* intVector::end() const {
    return _data + _size;
}

void intVector::resize(size_t newSize) {
    size_t newCapacity = _capacity;
    if (_capacity < newSize) {
        newCapacity = std::max(newSize, _capacity * 2);
        reallocate(newCapacity);
    }
    if (newSize > _size) {
        std::fill(_data + _size, _data + newSize, 0);
    }
    _size = newSize;
}

void intVector::swap(intVector& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
}