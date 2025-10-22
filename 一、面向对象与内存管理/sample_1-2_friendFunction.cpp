#include <iostream>
#include <cstdio>
#include <cstring>

class Adventurer {
private:
    char *name;
    int level;
public:
    Adventurer(const char *n, int l) : level(l) {
        name = new char(strlen(n) + 1);
        strcpy(name, n);
    }
    ~Adventurer() {
        delete[] name;
    }
    Adventurer(const Adventurer &other) : level(other.level) {
        name = new char(strlen(other.name) + 1);
        strcpy(name, other.name);
    }
    Adventurer& operator = (const Adventurer &other) {
        if (this == &other) return *this;
        delete[] name;
        name = new char(strlen(other.name) + 1);
        level = other.level;
        strcpy(name, other.name);
        return *this;
    }
    friend void printInfo(const Adventurer &adven);
};

void printInfo(const Adventurer &adven) {
    std::cout << "Name: " << adven.name << ", Level: " << adven.level << '\n';
}

int main() {
    Adventurer adv1("Hero", 10);
    printInfo(adv1);

    Adventurer adv2("Warrior", 20);
    printInfo(adv2);

    return 0;
}