#include <iostream>

class sample {
public:
    int data;
    char buffer[256];
    sample() {
        std::cout << "Constructor called." << std::endl;
    }

    ~sample() {
        std::cout << "Destructor called." << std::endl;
    }

    static void* operator new(size_t size) {
        std::cout << "Custom new operator called. Size: " << size << '\n';
        void* p = malloc(size);
        return p;
    }

    static void operator delete(void* p) {
        std::cout << "Custom delete operator called." << '\n';
        free(p);
    }
};

int main() {
    sample* obj = new sample;
    delete obj;
    return 0;
}