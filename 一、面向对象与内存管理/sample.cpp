#include <iostream>
#include <cstring>
#include <utility>

class Buffer {
private:
    char* data_;
    size_t size_;
public:
    // 构造：分配资源（RAII）
    Buffer(size_t sz = 0) : data_(nullptr), size_(sz) {
        if (size_) data_ = new char[size_];
        std::cout << "Buffer(size=" << size_ << ") constructed\n";
    }

    // 析构：释放资源
    ~Buffer() {
        delete[] data_;
        std::cout << "Buffer destroyed\n";
    }

    // 拷贝构造（深拷贝）
    Buffer(const Buffer& other) : data_(nullptr), size_(other.size_) {
        if (size_) {
            data_ = new char[size_];
            std::memcpy(data_, other.data_, size_);
        }
        std::cout << "Buffer copied (deep)\n";
    }

    // 拷贝赋值（遵循拷贝-交换惯用法）
    Buffer& operator=(Buffer other) { // 注意：参数按值，会调用拷贝或移动构造
        swap(*this, other);
        std::cout << "Buffer copy-assigned (via copy-and-swap)\n";
        return *this;
    }

    // 移动构造（高效转移资源）
    Buffer(Buffer&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
        std::cout << "Buffer moved\n";
    }

    // 交换函数（辅助拷贝-交换）
    friend void swap(Buffer& a, Buffer& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.size_, b.size_);
    }

    // 输出大小（示例 const 成员函数）
    size_t size() const noexcept { return size_; }

    // 写入（示例）
    void write(const char* src, size_t n) {
        if (n > size_) throw std::out_of_range("write overflow");
        std::memcpy(data_, src, n);
    }

    // 流输出重载（友元：访问私有）
    friend std::ostream& operator<<(std::ostream& os, const Buffer& b) {
        os << "Buffer(size=" << b.size_ << ")";
        return os;
    }
};

// 基类带虚函数（演示多态与 vtable）
class Animal {
public:
    virtual ~Animal() = default;
    virtual void speak() const {
        std::cout << "Animal sound\n";
    }
};

// 派生类重写虚函数
class Dog : public Animal {
public:
    void speak() const override {
        std::cout << "Woof\n";
    }
};

int main() {
    {
        Buffer b1(16);                 // 构造
        const char* msg = "hello";
        b1.write(msg, 6);
        std::cout << b1 << "\n";

        Buffer b2 = b1;                // 拷贝构造（深拷贝）
        Buffer b3 = std::move(b1);     // 移动构造
        b2 = b3;                       // 拷贝赋值（通过拷贝-交换）
    } // b2, b3, b1 析构（RAII）

    // 多态演示
    Animal* a = new Dog(); // 注意：使用 new 的示例（实际建议使用智能指针）
    a->speak();            // 运行时选择 Dog::speak()（通过 vtable）
    delete a;

    return 0;
}
