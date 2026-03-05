#include <new>
#include <iostream>

struct Data { int x; };

int main() {
    char buffer[sizeof(Data)];
    Data* p = new (buffer) Data{42};
    std::cout << p->x << '\n';
    p->~Data();
}
