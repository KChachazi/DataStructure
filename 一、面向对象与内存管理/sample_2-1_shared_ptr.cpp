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

int main() {
    std::shared_ptr<Sample> ptr1 = std::make_shared<Sample>(42);

    {
        std::shared_ptr<Sample> ptr2 = ptr1; // Shared ownership
        std::cout << "Reference Count: " << ptr1.use_count() << '\n';
        ptr2->display();
    } // ptr2 goes out of scope here
    
    std::cout << "Reference Count after ptr2 is out of scope: " << ptr1.use_count() << '\n';
}