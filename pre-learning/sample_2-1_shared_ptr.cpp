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
        std::shared_ptr<Sample> ptr3(ptr1);  // Another way to share ownership
        std::cout << "Reference Count: " << ptr1.use_count() << '\n';
        ptr2->display();
        ptr3->display();
    } // ptr2, ptr3 goes out of scope here

    std::cout << "Reference Count after ptr2 is out of scope: " << ptr1.use_count() << '\n';

    std::shared_ptr<Sample> ptr4 = ptr1;            // Shared ownership again
    std::shared_ptr<Sample> p = std::move(ptr1);    // Transfer ownership to p
    std::cout << "Reference Count after move(Count from p): " << p.use_count() << '\n';
    std::cout << "Reference Count after move(Count from ptr4): " << ptr4.use_count() << '\n';
    std::cout << "Is ptr1 null after move? " << (ptr1 == nullptr ? "Yes" : "No") << '\n';

    return 0;
}