#include <iostream>

class Sample {
public:
    Sample() : data(new int[100]) {
        std::cout << "Constructor called\n";
    }

    // 移动构造函数
    Sample(Sample&& other) noexcept : data(other.data) {
        other.data = nullptr; // 将其他对象的数据指针置为空，防止双重释放
        std::cout << "Move constructor called\n";
    }

    ~Sample() {
        delete[] data;
        std::cout << "Destructor called\n";
    }

    void display(std::string msg) const {
        std::cout << msg;
        if (data) {
            std::cout << "Data is " << data << '\n';
        } else {
            std::cout << "Data is null\n";
        }
    }
private:
    int* data;
};

int main()
{
    Sample obj1;
    Sample obj2 = std::move(obj1);
    obj1.display("After move, obj1: ");
    obj2.display("After move, obj2: ");
    return 0;
}