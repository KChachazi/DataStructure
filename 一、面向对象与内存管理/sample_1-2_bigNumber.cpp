#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>

class bigNumber {
    std::string number;
public:
    bigNumber(const std::string &num) : number(num) {}
    bigNumber(const int &num) {
        // 此处可以经过模板特化处理其他类型，挖坑等后续补充
        number = std::to_string(num);
    }

    bigNumber& operator =(const bigNumber &other) {
        if (this == &other) return *this;
        number = other.number;
        return *this;
    }

    bigNumber operator +(const bigNumber &other) const {
        std::string result;
        int carry = 0;
        int i = number.length() - 1, j = other.number.length() - 1;
        for (; i >= 0 || j >= 0 || carry; i --, j --) {
            int a = (i >= 0) ? number[i] - '0' : 0, b = (j >= 0) ? other.number[j] - '0' : 0;
            int sum = a + b + carry;
            carry = sum / 10;
            result += (sum % 10) + '0';
        }
        std::reverse(result.begin(), result.end());
        return bigNumber(result);
    }

    std::string getNumber() const {
        return number;
    }

    ~bigNumber() {}
};

bool operator == (const bigNumber &a, const bigNumber &b) {
    return a.getNumber() == b.getNumber();
}

bool operator < (const bigNumber &a, const bigNumber &b) {
    if (a.getNumber().length() != b.getNumber().length()) {
        return a.getNumber().length() < b.getNumber().length();
    }
    return a.getNumber() < b.getNumber();
}

bool operator <= (const bigNumber &a, const bigNumber &b) {
    return (a < b) || (a == b);
}

bool operator > (const bigNumber &a, const bigNumber &b) {
    return !(a <= b);
}

std::ostream& operator << (std::ostream &out, const bigNumber &num) {
    out << num.getNumber();
    return out;
} 

int main() {
    bigNumber num1("123456789012345678901234567890");
    bigNumber num2("987654321098765432109876543210");
    bigNumber sum = num1 + num2;
    std::cout << "Sum: " << sum << std::endl;

    std::cout << "Comparison(<): " << (num1 < num2) << std::endl;
    std::cout << "Comparison(==): " << (num1 == num2) << std::endl;

    return 0;
}