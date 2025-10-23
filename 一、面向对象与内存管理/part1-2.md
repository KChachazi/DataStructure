# Updates

续```part1-1.md```继续写

- 2025/10/23: 完成part1-2高级面向对象部分。

前面文章涉及了基本的面向对象的四大核心，同时也重点讨论了它们在C++中的实现方式，接下来将对C++ class的细节和行为控制进行进一步的学习探索。

# 面向对象与内存管理

## 深拷贝与浅拷贝

之前文章提及了C++类的构造函数和析构函数，这是进行类初始化的基本操作。然而需要注意的一点是，在复杂工程中经常会出现拷贝的问题，而仅靠默认的构造/析构函数容易导致隐性错误的出现，进而编译正常，但运行时错误。为了尽可能避免隐性错误的出现，对拷贝方式进行学习与讨论，进而灵活设计拷贝方式是非常重要的。

### 默认下的拷贝

在C++中，默认的拷贝方式一般是浅拷贝。

### 浅拷贝

浅拷贝非常简单，顾名思义就是进行浅层数据的拷贝。（此处笔者本来想要引入左值和右值的含义，不过详细展开需要单独的一节，因此也先挖坑）

#### 浅拷贝的实现

来看如下的一个简单类A：

```cpp
class A {
public:
    int val;
    char* name;
    A(int v, const char* n) : val(v) {
        name = new char[strlen(n) + 1];
        strcpy(name, n);
    }
    ~A() {
        delete[] name;
    }
};
```

我们执行下面的代码段：
```cpp
A a1(10, "Object A1");
A a2 = a1;
std::cout << "a1.val: " << a1.val << ", a1.name: " << a1.name << std::endl;
std::cout << "a2.val: " << a2.val << ", a2.name: " << a2.name << std::endl;

strcpy(a2.name, "Modified A2");
std::cout << "After modifying a2.name:" << std::endl;
std::cout << "a1.val: " << a1.val << ", a1.name: " << a1.name << std::endl;
std::cout << "a2.val: " << a2.val << ", a2.name: " << a2.name << std::endl;
```

得到的输出如下：
```
a1.val: 10, a1.name: Object A1
a2.val: 10, a2.name: Object A1
After modifying a2.name:
a1.val: 10, a1.name: Modified A2
a2.val: 10, a2.name: Modified A2
```

可以发现，当我们对```a2.name```进行修改后，```a1.name```也随之改变。这是由于C++的默认拷贝方式为浅拷贝，即对于基本数据类型使用赋值的方式直接拷贝，因此对于基本数据类型的指针，拷贝得到的是相同的地址。因而两个指针指向了相同的内存地址。

#### 浅拷贝的风险

对于上面的类A，首先发现的风险在前面已经出现了，就是两个类的数据会出现共享的情况，在庞大的代码中可能出现不可控的风险。

更严重的事，我们如果执行下面的代码：

```cpp
A *a1 = new A(10, "Original A1");
A *a2 = new A(*a1);
delete a1;
std::cout << a2->val << ", " << a2->name << '\n';
delete a2;
```

当我们主动释放```a1```对应的空间后，```a2```中的```name```变量也随之被主动释放，接着变成了野指针。随后我们主动释放```a2```时，还有多重释放的问题。

从上面对于该实例的分析中可以得到，浅拷贝只能对浅层数据进行拷贝。对于一些非指针的基本类型，浅拷贝不具有安全问题；但是对于指针和各种类（尤其包含原始指针、文件句柄等资源的类），浅拷贝会导致多个变量共享同一块内存的隐性问题。这种隐性问题同样是编译时不会出现，但是运行时导致错误的，包括但不限于：

- 双重释放
- 内存泄漏
- 悬空指针
- 数据意外共享

因此，在编写类的时候，我们需要主动控制对该类的拷贝方式以保证安全和可维护性。

### 深拷贝

深拷贝与浅拷贝相对，这种拷贝方式对深层数据也进行了拷贝。

#### 深拷贝的实现

如下是一种深拷贝的实现方式(遵循Rule of Three)：

```cpp
class A {
public:
    int val;
    char *name;
    A(int v, const char *n) : val(v) {
        name = new char[strlen(n) + 1];
        strcpy(name, n);
    }

    // 深拷贝构造函数
    A(const A &other) : val(other.val) {
        name = new char[strlen(other.name) + 1];
        strcpy(name, other.name);
    }

    // 深拷贝赋值运算符重载
    A& operator =(const A &other) {
        if (this == &other) return *this;

        delete[] name; // 释放已有资源
        val = other.val;
        name = new char[strlen(other.name) + 1];
        strcpy(name, other.name);
        return *this; // 返回自身以支持链式赋值 a = b = c;
    }

    // 析构函数
    ~A() {
        delete[] name;
    }
};
```

如上是一个典型的三法则深拷贝的实现方式。可以看到，在代码中通过对构造函数和赋值运算符的重载，我们成功将类中的指针成员分别指向了独立的内存地址，有效避免了浅拷贝中可能出现的一系列问题。

#### 评价深拷贝

容易看出，通过这种方式实现深拷贝，有效避免了浅拷贝可能出现的风险。深拷贝由于对深层数据进行拷贝，会增加很多时空成本；尽管两个对象拥有了互不干扰的独立数据，但是这也带来了更高的负担。

### 如何选择深拷贝/浅拷贝

浅拷贝通常用于无需管理动态内存的场景。如果仅仅是对于非动态内存数据(如基本数据类型等)进行拷贝，那么默认的浅拷贝就足够了。

此外，浅拷贝还适用于超大数据量但不进行数据修改的场景。浅拷贝可以有效避免过多的数据复制操作，从而节省资源的消耗。

对于其他更加灵活的场景，一般都选择深拷贝为优。诸如需要独立内存空间的，数据包含指针/其他资源时，对于每个对象都需要独立管理等等情况，深拷贝都是最安全使用的选择。

## 运算符重载

在设计一个新的类之后，经常会出现蹩脚的问题。原本的基本数据类型诸如```int, long, char```等等可以直接进行基本运算，与C++各种原生函数/流也可以直接适配；而新类则不能。因此，我们就会自然而然地考虑想要重新定义这些运算符，这就是运算符重载。

### 运算符重载的语法基础

基于先前对于深浅拷贝的讨论，我们已经实现了一种运算符重载。顺势我们可以介绍运算符重载的一般形式。

```cpp
返回类型 operator符号 (参数列表);
```

### 对一个```bigNumber```类进行运算符重载

接下来，我们尝试设计一个```bigNumber```类，用于在C++中实现高精度整数的运算。对```bigNumber```类的定义如下：

```cpp
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
    std::string getNumber() const {
        return number;
    }
    ~bigNumber() {}
};


```

#### 加法(减/乘/除法类似，高精度乘除后续可能会涉及)

```cpp
class A {
    /* ······前略······ */
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
}
```

此处演示的是在类中进行运算符重载。实际上可以发现，在类中实现运算符重载与重载虚函数大差不差。

#### 比较

```cpp
bool operator == (const bigNumber &a, const bigNumber &b) {
    return a.getNumber() == b.getNumber();
}

bool operator < (const bigNumber &a, const bigNumber &b) {
    if (a.getNumber().length() != b.getNumber().length()) {
        return a.getNumber().length() < b.getNumber().length();
    }
    return a.getNumber() < b.getNumber();
}
```

如上是在类外进行运算符重载的实例，在此处我们重载了```==, <```两个大小比较方式。

实际上，我们一般也的确是需要重载这两个符号即可。其余的大小比较方式都可以由这两种比较方式拼凑出来，比如：

```cpp
bool operator <= (const bigNumber &a, const bigNumber &b) {
    return (a < b) || (a == b);
}

bool operator > (const bigNumber &a, const bigNumber &b) {
    return !(a <= b);
}
```

#### 流输出

为了方便自己编写的类进行输出，我们可以对```<<```运算符重载，使其能够和```std::ostream```进行交互，直接输出。

```cpp
std::ostream& operator << (std::ostream &out, const bigNumber &num) {
    out << num.getNumber();
    return out;
}
```

### 运算符重载总览

C++中所有可以重载的运算符，以及不允许重载的运算符的完整列表见**文章末尾**。

此外需要注意的事，运算符重载有一定的限制：
- 不能改变运算符的优先级
- 不能改变运算符的结合性
- 不能改变运算符的操作数个数
- 不能创建新的运算符
- 至少有一个操作数是用户定义类型

### 如何进行高质量的运算符重载

经过上面的描述，我们认识到运算符重载是一个强大而实用的功能，而具有这种特点的功能一般又极有可能被滥用；因此，考虑什么运算符需要重载，在什么情景下重载，是一个非常严肃且重要的问题。

笔者经查阅资料，结合自身理解，得到下面的笼统简要概括。

#### 必须重载

有些运算符功能十分重要，在代码中地位关键，**定义了对象的基本行为语义**，一般**必须重载**。

| 运算符          | 常见形式                                                       | 使用场景 / 意义                      | 建议实现形式  |
| ------------ | ---------------------------------------------------------- | ------------------------------ | ------- |
| `operator=`  | `T& operator=(const T& other)`                               | 拷贝赋值，控制资源所有权（深拷贝）              | 成员函数    |
| `operator==` | `bool operator==(const T& other) const`                      | 判断两个对象是否“等价”                   | 成员函数或友元 |
| `operator!=` | 基于 `==`                                                    | 通常定义为 `return !(*this == other)` | 成员函数    |
| `operator<<` | `friend std::ostream& operator<<(std::ostream&, const T&)` | 支持 `std::cout << obj` 输出       | 友元函数    |
| `operator>>` | `friend std::istream& operator>>(std::istream&, T&)`       | 支持 `std::cin >> obj` 输入        | 友元函数    |

#### 功能性重载

根据类的实际意义，我们通常还要进行功能性重载。例如，对于向量类一般需要重载加减，矩阵需要重载加法和乘法等等。

| 运算符                                                | 场景示例                       | 使用语义         | 常见实现方式        |
| -------------------------------------------------- | -------------------------- | ------------ | ------------- |
| `operator+`, `operator-`, `operator*`, `operator/` | 数学类（Vector、Matrix、Complex） | 算术运算         | 成员或非成员函数      |
| `operator+=`, `operator-=` 等                       | 累加型类                       | 自修改运算        | 成员函数          |
| `operator[]`                                       | 容器类（Vector、Array）          | 元素访问（支持下标）   | 成员函数（返回引用）    |
| `operator()`                                       | 仿函数、可调用对象                  | 让对象“像函数一样调用” | 成员函数          |
| `operator<`, `operator>` 等                         | 有序类（Date、String）           | 比较排序         | 成员函数或友元       |
| `operator->`, `operator*`                          | 智能指针类                      | 模拟原生指针操作     | 成员函数          |
| `operator bool`                                    | 状态类（智能指针、文件流）              | 判断有效性        | 成员函数          |
| `operator++`, `operator--`                         | 迭代器、计数类                    | 自增/自减        | 成员函数（区分前置/后置） |

#### 可选/特殊情况重载

这一部分常常用来设计特定的语法糖或高阶接口，一般情况下不建议实现，除非进行库开发、模板适配、运算符重定向等。

| 运算符                                | 场景             | 示例                              | 注意事项            |
| ---------------------------------- | -------------- | ------------------------------- | --------------- |
| `operator new` / `operator delete` | 自定义内存池或分配器     | `MyClass::operator new(size_t)` | 谨慎使用；需配对 delete |
| `operator&`                        | 智能引用或特殊语义类     | 控制取址操作行为                        | 罕见              |
| `operator->*` / `operator*`        | 函数指针代理类        | 用于实现 delegate 模式                | 高级技巧            |
| `operator,`                        | 控制逗号运算         | 模拟链式操作                          | 容易误用            |
| `operator[]` 的非常规重载                | 矩阵多维访问         | `matrix[i][j]`                  | 可返回代理类          |
| `operator""`（字面量运算符）               | C++11 用户自定义字面量 | `10_km`, `"abc"_s`              | 现代语法特性          |
| `operator<=>`                      | C++20 三路比较     | 自动生成 `<`, `>`, `==`             | 仅在 C++20+       |

#### 建议原则

- 语义一致性
    运算符应当符合用户直觉。
    例如 a + b 应不修改 a，而 a += b 可修改。

- 避免滥用语法糖
    不要为“好看”而重载；如果语义不明显，会造成维护困难。

- 非成员优先
    当需要隐式类型转换时，优先定义为非成员函数（或友元）。

- 异常安全与 const 正确性
    只读操作符应声明为 const。赋值、算术操作应考虑资源释放与异常安全。

## 友元机制

先前提到，类的封装特性使得我们能够主动控制其内部成员是否可以被外部、派生类等访问，然而在实际开发中经常也会因此受限。相当多情况下，需要定义一些函数，这些函数不是类的一部分，但是需要频繁地访问类的私有/保护(```private/protected```)成员，为了降低维护成本，C++引入了```friend```关键词，实现了**受控破封机制**，允许非成员函数访问类的私有成员。

借助友元，可以使得普通函数或其他类中的成员函数可以访问某个类的私有成员和保护成员。

- 友元函数：普通函数可以访问某个类私有成员或保护成员。
- 友元类：类A中的成员函数可以访问类B中的私有或保护成员。

### 友元函数

友元函数本身不是类的成员函数，但是可以访问某个类私有成员或保护成员。

```cpp
class Adventurer {
private:
    char *name;
    int level;
public:
    Adventurer(const char *n, int l) : level(l) {
        name = new char[strlen(n) + 1];
        strcpy(name, n);
    }
    ~Adventurer() {
        delete[] name;
    }
    Adventurer(const Adventurer &other) : level(other.level) {
        name = new char[strlen(other.name) + 1];
        strcpy(name, other.name);
    }
    Adventurer& operator = (const Adventurer &other) {
        if (this == &other) return *this;
        delete[] name;
        name = new char[strlen(other.name) + 1];
        level = other.level;
        strcpy(name, other.name);
        return *this;
    }
    friend void printInfo(const Adventurer &adven);
};

void printInfo(const Adventurer &adven) {
    std::cout << "Name: " << adven.name << ", Level: " << adven.level << '\n';
}
```

对于上面的```Adventurer```类，我们运行下面的代码：

```cpp
Adventurer adv1("Hero", 10);
printInfo(adv1);
Adventurer adv2("Warrior", 20);
printInfo(adv2);
```

输出如下：

```
Name: Hero, Level: 10
Name: Warrior, Level: 20
```

可以看出，尽管```printInfo()```函数不是```Adventurer```类，但是仍然可以访问其私有/保护成员。

### 友元类

类似地，我们也可以声明友元类。

```cpp
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
```

对于如上的实例，我们执行下面的代码：

```cpp
AdventurerManager manager;
manager.addAdventurer("Alice", 10);
manager.addAdventurer("Bob", 20);
manager.addAdventurer("Charlie", 15);
manager.printAllAdventurerInfo();

manager.deleteAdventurer("Alice");
std::cout << "After deletion:" << '\n';
manager.printAllAdventurerInfo();
```

进而我们得到下面的输出：

```
Name: Alice, Level: 10
Name: Bob, Level: 20
Name: Charlie, Level: 15
After deletion:
Name: Bob, Level: 20
Name: Charlie, Level: 15
```

可见，在```AdventurerManager```类中我们顺利调用了```Adventurer```类私有的构造函数。

实际上上面的实例是一个简单的**工厂模型**：```Adventurer```类无法主动被声明，其完全只能在```AdventurerManager```类中生成，就像产品只能在工厂中生产出来一样。

此外请注意，**此处的私有构造函数是特殊情况，一般情况下不需要这么写**。

### 友元的适用情况

友元也是一种强大而使用的功能，使用不当就会破坏类的独立性。因此同样地，和学习如何使用友元一样重要的是知道什么情境下适合使用友元。

可能的情境如下：

- 需要对称性的运算符重载：当运算符的左操作数不是当前类对象时，一般使用友元。
- 紧密协作的类(桥接模式)：两个类紧密协作，相互访问私有成员。
- 工厂模型
- 测试/访问类

无论何种情境，我们在使用友元时，都应该最小化友元的使用，尽可能限制友元在局部类中使用，并且将使用友元的原因注明以便维护。

## 静态成员与常量成员

类的设计是为了方便管理一类对象，我们设计类时最关注的一般也都是它们的共同特性。因此对于某些特殊的共同特性，C++也引入了关键词```static, const```用来维护这些特殊的成员。

### 静态成员(**static**)

**static**关键字本身在C语言中存在，在C++面向对象中增加了一种完全不同的功能。在类中，**static**修饰的成员称为静态成员。

> 静态成员是类的属性，而不是类生成的任何一个对象的属性。

我们通过一个实例来理解上面这句话。

```cpp
class A {
public:
    static int staticVar;
    static void staticMethod() {
        std::cout << "Static Method called. Static Variable: " << staticVar << std::endl;
    }
};
int A::staticVar = 0;
```

注意，上面的```int A::staticVar = 0;```是必须的，因为静态成员变量属于类而不是类的某个对象，所以需要在类外进行定义和初始化。这一步之所以必须是因为在类中该变量进行了声明，但是没有定义，在内存中不存在其位置，进而链接器找不到其定义。

在C++17及之后的标准中，可以在类内添加**inline**关键字，在类中直接通过```inline int A::staticVar = 0;```进行初始化，详细编译原理可以自行查询。

如果要访问类的静态成员，可以参考下面的语句。

```cpp
A::staticVar = 10;
A::staticMethod();
```

从上面的访问语句中也可看出，静态成员本质上是类的属性，而不是类生成的某个对象的属性。

此外，有关静态成员有几点注意事项：
- 如果从类生成的某个对象中访问其静态成员，则所有对象共享该类的静态成员。
- 静态成员函数没有```this```指针，因而无法访问类的非静态成员。
- 编译器会在**const**成员函数中阻止对任何非**mutable**成员的修改。(**mutable**在此不作展开)

### 常量成员(**const**)

在编码时，有些变量是常量，不希望被修改；有些函数仅访问，不允许修改对象状态。因而，**const**关键字出现了。

**const**关键字的作用在于：
- 用于声明不应修改的对象或方法。
- 常量成员函数（在函数名后加```const```）表示该函数不会(也不能)改变对象状态。

具体来看下面的例子：

```cpp
class Adventurer {
private:
    const int id_;
    int level_;
public:
    Adventurer(int id, int level) : id_(id), level_(level) {}

    int getID() const { return id_; }
    void setLevel(int l) { level_ = l; }
};
```

在这个例子中，一旦一个```Adventurer```类的对象经过构造函数生成，随后其内部成员```const int id_;```的值将永不再变化。

另：```#define```在部分情况下也能实现这种功能，但是区别在于**const**关键字同时定义了数据类型，因而更精确；**define**则只是简单的文本替换。

## 附：C++ 可重载运算符完整表格

| 类别 | 运算符 | 名称 | 语法示例 | 备注 |
|------|--------|------|----------|------|
| **算术运算符** | `+` | 加法 | `a + b` | 一元和二元都可重载 |
| | `-` | 减法/取负 | `a - b`, `-a` | 一元和二元都可重载 |
| | `*` | 乘法 | `a * b` | |
| | `/` | 除法 | `a / b` | |
| | `%` | 取模 | `a % b` | |
| **自增自减** | `++` | 自增 | `++a`, `a++` | 前缀和后缀版本 |
| | `--` | 自减 | `--a`, `a--` | 前缀和后缀版本 |
| **位运算符** | `&` | 位与 | `a & b` | 也可用作取地址（一元） |
| | `\|` | 位或 | `a \| b` | |
| | `^` | 位异或 | `a ^ b` | |
| | `~` | 位取反 | `~a` | 一元运算符 |
| | `<<` | 左移 | `a << b` | 也用于输出流 |
| | `>>` | 右移 | `a >> b` | 也用于输入流 |
| **赋值运算符** | `=` | 赋值 | `a = b` | **必须为成员函数** |
| | `+=` | 加赋值 | `a += b` | |
| | `-=` | 减赋值 | `a -= b` | |
| | `*=` | 乘赋值 | `a *= b` | |
| | `/=` | 除赋值 | `a /= b` | |
| | `%=` | 取模赋值 | `a %= b` | |
| | `&=` | 位与赋值 | `a &= b` | |
| | <code>&#124;=</code> | 位或赋值 | <code>a &#124;= b</code> | |
| | `^=` | 位异或赋值 | `a ^= b` | |
| | `<<=` | 左移赋值 | `a <<= b` | |
| | `>>=` | 右移赋值 | `a >>= b` | |
| **比较运算符** | `==` | 等于 | `a == b` | |
| | `!=` | 不等于 | `a != b` | |
| | `<` | 小于 | `a < b` | |
| | `>` | 大于 | `a > b` | |
| | `<=` | 小于等于 | `a <= b` | |
| | `>=` | 大于等于 | `a >= b` | |
| **逻辑运算符** | `!` | 逻辑非 | `!a` | 一元运算符 |
| | `&&` | 逻辑与 | `a && b` | |
| | `\|\|` | 逻辑或 | `a \|\| b` | |
| **成员访问** | `->` | 成员访问 | `a->b` | **必须为成员函数** |
| | `->*` | 成员指针访问 | `a->*b` | |
| **下标** | `[]` | 下标 | `a[b]` | **必须为成员函数** |
| **函数调用** | `()` | 函数调用 | `a(b, c)` | **必须为成员函数** |
| **类型转换** | `operator type()` | 类型转换 | `(type)a` | **必须为成员函数**，无返回类型 |
| **内存管理** | `new` | 动态内存分配 | `new A` | 必须是静态成员函数 |
| | `new[]` | 动态数组分配 | `new A[n]` | 必须是静态成员函数 |
| | `delete` | 内存释放 | `delete a` | 必须是静态成员函数 |
| | `delete[]` | 数组内存释放 | `delete[] a` | 必须是静态成员函数 |
| **其他** | `,` | 逗号 | `a, b` | |
| | `&` | 取地址 | `&a` | 一元运算符 |

#### 不允许重载的运算符

| 运算符 | 名称 | 不允许重载的原因 |
|--------|------|------------------|
| `::` | 作用域解析 | 语法核心，必须保持原有语义 |
| `.` | 成员访问 | 保证成员访问的安全性 |
| `.*` | 成员指针访问 | 与`.`类似，保证访问安全 |
| `?:` | 条件运算符 | 语法过于复杂，可能引起混淆 |
| `sizeof` | 大小运算符 | 必须返回对象的确切大小 |
| `alignof` | 对齐查询 | 必须返回正确的对齐值 |
| `typeid` | 类型信息 | 必须返回真实的类型信息 |
| `static_cast` | 静态类型转换 | 类型系统核心操作 |
| `dynamic_cast` | 动态类型转换 | 运行时类型信息核心操作 |
| `const_cast` | const转换 | 常量性修改核心操作 |
| `reinterpret_cast` | 重解释转换 | 低层类型转换核心操作 |