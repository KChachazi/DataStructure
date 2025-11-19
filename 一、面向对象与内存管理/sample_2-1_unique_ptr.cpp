#include <iostream>
#include <memory>

class Sample {
    int data;
public:
    Sample() { std::cout << "Sample Constructor" << '\n'; }
    Sample(int val) : data(val) { std::cout << "Sample Parameterized Constructor" << '\n'; }
    ~Sample() { std::cout << "Sample Destructor" << '\n'; }
    void display() {
        std::cout << "Display Sample" << '\n';
        std::cout << "Data: " << data << '\n';
    }
};

int main()
{
    std::unique_ptr<Sample> ptr1 = std::make_unique<Sample>(1);
    ptr1->display();
    std::unique_ptr<Sample> ptr2 = std::make_unique<Sample>(2);
    ptr2->display();

    std::swap(ptr1, ptr2);

    ptr1->display();
    ptr2->display();

    ptr1.reset(ptr2.release());
    
    return 0;
}