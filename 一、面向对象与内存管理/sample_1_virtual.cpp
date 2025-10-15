#include <iostream>
#include <cstdio>
#include <cstring>

class Animal {
    std::string name_;
    int age_;
public:
    Animal(const char *name, int age) : name_(name), age_(age) {
        std::cout << "Animal " << name_ << " created.\n";
    }

    virtual ~Animal() {
        std::cout << "Animal " << name_ << " destroyed.\n";
    }

    virtual void makeSound() const {
        std::cout << "Animal sound!\n";
    }

    const std::string getName() const {
        return name_;
    }
};

class Dog : public Animal {
public:
    Dog(const char *name, int age) : Animal(name, age) {
        std::cout << "Dog " << name << " created.\n";
    }

    ~Dog() override {
        std::cout << "Dog destroyed.\n";
    }

    void makeSound() const override {
        std::cout << "Woof! Woof!\n";
    }
};

int main() {
    Animal *a = new Animal("Generic", 5);
    a->makeSound();
    delete a;

    Animal *d = new Dog("Buddy", 3);
    d->makeSound();
    delete d;

    return 0;
}