#include <iostream>
#include <cstdio>

class areaCalculator {
public:
    // 纯虚函数
    virtual double getArea() const = 0;
    // 虚析构函数
    virtual ~areaCalculator() = default;
};

class shape {
protected:
    double width;
    double height;
public:
    shape(double w, double h) : width(w), height(h) {}
    virtual ~shape() = default;
};

class rectangle : public shape, public areaCalculator {
public:
    rectangle(double w, double h) : shape(w, h) {}
    double getArea() const override {
        return width * height;
    }
    ~rectangle() = default;
};

int main() {
    areaCalculator* ac = nullptr;
    // ac = new areaCalculator(); // 错误：不能实例化抽象类

    rectangle* rect = new rectangle(5.0, 3.0);
    std::cout << "Area of rectangle: " << rect->getArea() << '\n';
    delete rect;
    return 0;
}