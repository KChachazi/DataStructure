#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>

class AdventurerManager; // 前向声明

class Adventurer {
private:
    char *name;
    int level;
    Adventurer(const char *n, int l) : level(l) {
        name = new char[strlen(n) + 1];
        strcpy(name, n);
    }
    ~Adventurer() {
        delete[] name;
    }
public:
    // 声明友元函数
    friend class AdventurerManager;
};

class AdventurerManager {
private:
    std::vector<Adventurer*> adventurers;
public:
    AdventurerManager() {}
    ~AdventurerManager() {
        for (auto adv : adventurers) {
            delete adv;
        }
    }

    void addAdventurer(const char *name, int level) {
        Adventurer *adven = new Adventurer(name, level);
        adventurers.push_back(adven);
    }

    void deleteAdventurer(const char *name) {
        for (auto it = adventurers.begin(); it != adventurers.end(); it ++) {
            if (strcmp((*it)->name, name) == 0) {
                delete *it;
                adventurers.erase(it);
                break;
            }
        }
    }
    
    void printAdventurerInfo(const Adventurer &adv) {
        std::cout << "Name: " << adv.name << ", Level: " << adv.level << std::endl;
    }

    void printAllAdventurerInfo() {
        for (const auto &adv : adventurers)
            printAdventurerInfo(*adv);
    }
};


int main() {
    AdventurerManager manager;
    manager.addAdventurer("Alice", 10);
    manager.addAdventurer("Bob", 20);
    manager.addAdventurer("Charlie", 15);
    manager.printAllAdventurerInfo();

    manager.deleteAdventurer("Alice");
    std::cout << "After deletion:" << '\n';
    manager.printAllAdventurerInfo();

    return 0;
}