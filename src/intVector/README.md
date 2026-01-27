AI建议的可能目录，后续可能有修改

## 1. Vector 的设计基础

vector 的三段式内存模型

_begin, _end, _cap

size 与 capacity 的定义

vector 提供的核心功能（标准要求概览）

## 2. 内存管理与构造机制

动态内存：operator new / operator delete

placement-new：构造对象

显式析构：ptr->~T()

allocator 的基本思想（可选）

## 3. 动态扩容策略

growth factor（通常 ×2）

扩容的实现步骤

移动构造 vs 深拷贝

扩容带来的迭代器失效

## 4. 模板类实现 vector

基础成员变量

构造函数（默认、带 size、带初值）

析构函数

push_back / pop_back

emplace_back（perfect forwarding）

## 5. 拷贝与移动语义

拷贝构造与深拷贝

移动构造（O(1)指针偷取）

拷贝赋值（copy-and-swap）

移动赋值（swap trick）

swap 成员函数设计

## 6. 异常安全保证

强保证 vs 基本保证 vs no-throw

哪些操作需要强保证？

push_back

resize

reallocate

如何实现强保证？

临时 buffer

rollback 技术

noexcept 的重要性

## 7. 迭代器设计

随机访问迭代器的分类

裸指针作为迭代器

begin / end 的实现

const_iterator

迭代器失效规则

## 8. 其他典型接口实现

operator[] 与 at()

reserve / capacity / shrink_to_fit

resize

insert / erase（难点）

clear