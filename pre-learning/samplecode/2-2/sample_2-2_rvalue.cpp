#include <iostream>

int main()
{
    std::string str = "Hello, World!";
    std::cout << "Original string: " << str << '\n';
    std::string t = std::move(str); // 使用 std::move 将 str 转换为右值引用
    std::cout << "Moved string: " << t << '\n';
    std::cout << "Original string after move: " << str << '\n'; // str 的状态未定义，通常为空
    return 0;
}