#include <iostream>

class Box {
public:
    Box(int v) : value(v) {
        std::cout << "Box constructed: " << value << '\n';
    }
    ~Box() {
        std::cout << "Box destructed: " << value << '\n';
    }
private:
    int value;
};

int main() {
    std::cout << "=== 栈上对象 ===\n";
    Box stackBox(1);

    std::cout << "=== 堆上对象 ===\n";
    Box* heapBox = new Box(2);

    std::cout << "=== 程序结束 ===\n";
    delete heapBox;
}
