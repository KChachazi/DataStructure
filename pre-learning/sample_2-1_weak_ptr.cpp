#include <iostream>
#include <memory>

class Sample {
    int data;
public:
    Sample() { std::cout << "Sample Constructor" << '\n'; }
    Sample(int val) : data(val) { std::cout << "Sample Parameterized Constructor" << '\n'; }
    ~Sample() { std::cout << "Sample Destructor" << '\n'; }
    void display() {
        std::cout << "Display Sample -> Data: " << data << '\n';
    }
};

int main() {
    std::shared_ptr<Sample> sptr1 = std::make_shared<Sample>(1);
    sptr1->display();
    std::shared_ptr<Sample> sptr2 = std::make_shared<Sample>(2);
    sptr2->display();
    std::weak_ptr<Sample> wptr1 = sptr1;
    std::weak_ptr<Sample> wptr2 = sptr2;

    if (auto spt = wptr1.lock()) {
        std::cout << "using weak_ptr to access shared_ptr data: ";
        spt->display();
    } else {
        std::cout << "wptr1 is expired" << '\n';
    }
    if (auto spt = wptr2.lock()) {
        std::cout << "using weak_ptr to access shared_ptr data: ";
        spt->display();
    } else {
        std::cout << "wptr2 is expired" << '\n';
    }

    return 0;
}