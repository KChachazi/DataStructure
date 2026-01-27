#include <iostream>
#include <vector>

template<typename T>
vector<T> createVector(size_t size, const T& initialValue) {
    vector<T> vec(size, initialValue);
    return vec;
}

int main() {

}