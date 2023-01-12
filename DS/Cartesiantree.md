### 一、模板类别

​	数据结构：笛卡尔树

### 二、模板功能

#### 1.建立笛卡尔树

1. 数据类型

   模板参数 `typename _Compare` ，表示比较函数的类，默认为小于号函数。

   模板参数 `uint32_t _MAXN` ，表示区间的最大长度，默认为 `1<<20` 。

   构造参数 `_Iterator __first` ，表示区间首位置的迭代器。

   构造参数 `_Iterator __last` ，表示区间尾位置的迭代器。
   
   构造参数 `_Compare __comp` ，表示具体的比较函数。默认为 `_Compare` 类的默认实例。本参数接收类型有：普通函数，函数指针，仿函数，匿名函数，泛型函数。

2. 时间复杂度

   $O(n)$ 。

3. 备注

   以下讨论，假设比较函数是小于号函数，笛卡尔树维护区间最大值。

   笛卡尔树作为二叉树，满足以下两个性质：

   1. 树的中序遍历，就是原区间顺序；

   2. 相邻的元素中，较小的元素总是较大的元素的子孙。

   显然，要做到这一点，我们要首先挑出整个区间中的最大元素作为根，同时将整个区间划分成两半；在分割出的子区间里，继续挑出最大元素，作为根的左/右孩子，同时将子区间继续分割成两半...
   
   笛卡尔树​表处理的问题为区间最值查询，可以是区间最大值、区间最小值。通过上述分析，可以得知，区间最值就是区间最左元素和最右元素在树中的 `LCA` 。

#### 2.查询区间最值元素

1. 数据类型

   输入参数 `uint32_t __a`​ ，表示区间一端的下标。

   输入参数 `uint32_t __b`​ ，表示区间另一端的下标。

   返回类型 `uint32_t`​ ，表示区间最值所在的下标。

2. 时间复杂度

   $O(\log n)$。

3. 备注

   本方法通过轻重链剖分的方法进行 `LCA` 查询。

   **注意：**查询得到的结果并不是区间最值，而是区间最值元素所在的下标。


### 三、模板示例

```c++
#include "DS/Cartesiantree.h"
#include "IO/FastIO.h"

int main() {
    //先给出一个长度为 10 的数组
    int A[10] = {11, 5, 9, 12, 8, 4, 6, 15, 7, 7};
    for (int i = 0; i < 10; i++)
        cout << A[i] << (i == 9 ? '\n' : ' ');

    //建立一个区间最大值笛卡尔树
    auto mymax = [](int x, int y) {
        return x > y ? x : y;
    };
    //一般可以忽略模板参数
    OY::Cartesiantree tree_max(A, A + 10);

    cout << "max(A[3~6])       =" << A[tree_max.query(3, 6)] << endl;

    cout << "max(A[4~8])'s pos =" << tree_max.query(4, 8) << endl;
}
```

```
#输出如下
11 5 9 12 8 4 6 15 7 7
max(A[3~6])       =12
max(A[4~8])'s pos =7

```
