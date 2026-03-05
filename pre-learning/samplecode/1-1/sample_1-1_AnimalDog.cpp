#include <iostream>
#include <cstdio>
#include <cstring>

class Animal {
private:
    char *name_;
    int age_;
public:
    Animal(const char *name, int age) : age_(age) {
        name_ = new char[strlen(name) + 1];
        std::strcpy(name_, name);
        std::cout << "Animal " << name_ << " created.\n";
    }

    ~Animal() {
        std::cout << "Animal " << name_ << " destroyed.\n";
        delete[] name_;
    }

    char* getName() const {
        return name_;
    }
    void eat() const {
        std::cout << name_ << " is eating.\n";
    }
    void sleep() const {
        std::cout << name_ << " is sleeping.\n";
    }
};

class Dog : public Animal {
public:

    Dog(const char *name, int age) : Animal(name, age) {
        std::cout << "Dog " << name << " created.\n";
    }
    
    void bark() const {
        std::cout << "Woof! Woof!\n";
    }
};

int main() {
    Dog *dog = new Dog("Pappu", 3);
    std::cout << "Dog's name is: " << dog->getName() << "\n";
    dog->bark();
    delete dog;
    return 0;
}