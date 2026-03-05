#include <iostream>
#include <cstdio>

class Adventurer {
private:
    char *name_;
    int *level_, *health_, *attack_;

public:
    // 构造函数
    Adventurer(const char *name, int level, int health, int attack)
        : level_(new int(level)), health_(new int(health)), attack_(new int(attack)) {
        name_ = new char[strlen(name) + 1];
        std::strcpy(name_, name);
        std::cout << "Adventurer " << name_ << " created.\n";
    }

    // 析构函数
    ~Adventurer() {
        delete[] name_;
        delete level_;
        delete health_;
        delete attack_;
        std::cout << "Adventurer " << name_ << " destroyed.\n";
    }

    // 返回生命值
    int getHealth() const {
        return *health_;
        // const成员函数保证不修改对象状态
    }

    // 返回攻击力
    int getAttack() const {
        return *attack_;
    }

    // 返回等级
    int getLevel() const {
        return *level_;
    }

    // 返回名字
    const char* getName() const {
        return name_;
    }

    // 生命值变化
    void changeHealth(int delta) {
        *health_ += delta;
        if (*health_ < 0) *health_ = 0;
        std::cout << name_ << "'s health changed to " << *health_ << ".\n";
        if (*health_ == 0) {
            std::cout << name_ << " has been killed!\n";
            delete this; // 自我销毁
        }
    }

    // 攻击力变化
    void changeAttack(int delta) {
        *attack_ += delta;
        if (*attack_ < 0) *attack_ = 0;
        std::cout << name_ << "'s attack changed to " << *attack_ << ".\n";
    }

    // 显示状态
    void displayStatus() const {
        std::cout << "Adventurer: " << name_ << "\n"
                  << " Level: " << *level_ << "\n"
                  << " Health: " << *health_ << "\n"
                  << " Attack: " << *attack_ << "\n";
    }

protected:
    // 禁止拷贝构造和赋值
    Adventurer(const Adventurer&) = delete;
    Adventurer& operator=(const Adventurer&) = delete;
};