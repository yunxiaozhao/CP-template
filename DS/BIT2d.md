### 一、模板类别

​	数据结构：二维树状数组，二维树状数组加强版

### 二、模板功能

#### 1.建立二维树状数组

1. 数据类型

   模板参数 `typename _Tp` ，表示元素类型。

   模板参数 `typename _Plus`​ ，表示加和函数的类，默认为 `std::plus<_Tp>` ，也就是加法。

   模板参数 `typename _Minus` ，表示做差函数的类，默认为 `std::minus<_Tp>` ，也就是减法。

   构造参数 `int __row` ，表示二维树状数组的行范围为 `[0, __row)`。

   构造参数 `int __column` ，表示二维树状数组的列范围为 `[0, __column)`。

   构造参数 `_Plus __plus`​ ，表示具体的加和函数，默认值为 `_Plus` 类的默认实例。

   构造参数 `_Minus __minus`​ ，表示具体的做差函数，默认值为 `_Minus` 类的默认实例。

   构造参数 `_Tp __defaultValue` ，表示元素默认值，默认值为 `_Tp` 类的默认实例。

2. 时间复杂度

   $O(m*n)$ ，其中 `m` 和 `n` 分别表示行数和列数。

3. 备注

   二维树状数组处理的问题为单点修改，单点/区域和的查询。二维树状数组加强版可以同时处理单点/区域修改，单点/区域和的查询。当然，二维树状数组加强版的时空效率都会略逊于普通的二维树状数组。所以，如果只处理区域和查询而不进行区域修改，最好使用普通的二维树状数组；如果只处理区域修改和单点查询，最好对原二维数组的差分数组建立普通二维树状数组（二维差分有一定的难度）。

   二维树状数组要求加和函数的运算符满足**交换律**和**结合律**。

   二维树状数组要求加和函数和做差函数为**逆运算**，即：`plus(a,b)==c <=> minus(c,a)==b`。

   显然，二维树状数组非常适合重载加法和减法，或者重载乘法和除法。需要注意的是，重载加减法时，元素默认值传递为 `0` 较为合适；重载乘除法时，元素默认值传递为 `1` 较为合适。

   二维树状数组通过加和函数求解前缀和，再通过前缀和做差求解任意区域和。这就是为什么需要加和函数和做差函数两个函数。在一些情况下，也可以违反上述规则，比如将加和函数设为`std::max<_Tp>`。那么，显然找不到对应的做差函数，所以只能求解前缀最大值，不能求解任意区域最大值。

   二维树状数组加强版通过一些差分运算，实现了区域修改。对于加强版，因为应用范围较小，所以没有给出加和函数、做差函数、元素默认值的设置接口，默认为加法、减法、`0`。所以在构造函数中只需要传递区域大小这一个参数。

#### 2.建立二维树状数组

1. 数据类型

   构造参数 `Ref __ref` ，表示一个寻址函数，通过调用 `__ref(i,j)` 找到原矩阵的 `i` 行 `j` 列的元素。推荐使用匿名函数来传参。

   其它同上。

2. 时间复杂度

   同上。

3. 备注

   同上。

   使用迭代器进行初始化，可以将区间初状态直接赋到树状数组里。
   
   **注意：**寻址函数的类型和加和函数、做差函数恰好类似，均为二元函数。为了避免混淆，所以在传寻址函数的时候，需要把寻址函数放在 `__row` 和 `__column` 的前面。
   
   **注意：**在普通二维数组构造时，寻址函数既可以传二元函数，通过调用 `__ref(i,j)` 找到原矩阵的 `i` 行 `j` 列的元素；也可以传一元函数，通过调用 `__ref(i)` 找到原矩阵的 `i` 行的起始位置。


####  3.重置

1. 数据类型

   输入参数 `int __row` ，表示二维树状数组的行范围为 `[0, __row)`。

   输入参数 `int __column` ，表示二维树状数组的列范围为 `[0, __column)`。

2. 时间复杂度

   $O(m*n)$ 。

3. 备注

   调用本函数会将二维树状数组大小改变，并将之前的合并信息重置。

#### 4.重置

1. 数据类型

   输入参数 `Ref __ref` ，表示一个寻址函数，通过调用 `__ref(i,j)` 找到原矩阵的 `i` 行 `j` 列的元素。推荐使用匿名函数来传参。

   其它同上。

2. 时间复杂度

   同上。

3. 备注

   同上。

   使用迭代器进行重置，可以将区间初状态直接赋到二维树状数组里。

   **注意：**寻址函数要放在 `__row` 和 `__column` 的前面。

   **注意：**寻址函数既可以传二元函数，通过调用 `__ref(i,j)` 找到原矩阵的 `i` 行 `j` 列的元素；也可以传一元函数，通过调用 `__ref(i)` 找到原矩阵的 `i` 行的起始位置。

#### 5.单点增值

1. 数据类型

   输入参数 `int i` ，表示单点增值的行下标。

   输入参数 `int j` ，表示单点增值的列下标。

   输入参数 `_Tp __inc​` ，表示增量大小。

2. 时间复杂度

   $O(\log (m\cdot n))$ 。

#### 6.区域增值

1. 数据类型

   输入参数 `int __row1` ，表示区域增值的起始行下标。

   输入参数 `int __row2` ，表示区域增值的结束行下标。（闭区间）

   输入参数 `int __column1​` ，表示区域增值的起始列下标。

   输入参数 `int __column2` ，表示区域增值的结束列下标。（闭区间）

   输入参数 `_Tp __inc` ，表示增量大小。

2. 时间复杂度

   $O(\log (m\cdot n))$ 。

3. 备注

   本方法仅在二维树状数组加强版里存在。

#### 7.前缀和查询

1. 数据类型

   输入参数 `int i`，表示前缀和查询的结尾行下标。(闭区间)

   输入参数 `int j`，表示前缀和查询的结尾列下标。(闭区间)

2. 时间复杂度

   $O(\log(m\cdot n))$ 。
   
3. 备注

   虽然名义上为**前缀和**，但是实际上指的是位于点 $[i,\space j]$ 左上方的区域和。（包括 `i` 行 `j` 列）

#### 8.单点查询

1. 数据类型

   输入参数 `int i` ，表示单点查询的行下标。

   输入参数 `int j` ，表示单点查询的列下标。

2. 时间复杂度

   $O(\log(m\cdot n))$ 。

3. 备注

   本方法只有在做差函数合理的时候有意义。

#### 9.区域查询

1. 数据类型

   输入参数 `int __row1` ，表示区域查询的起始行下标。

   输入参数 `int __row2` ，表示区域查询的结束行下标。（闭区间）

   输入参数 `int __column1​` ，表示区域查询的起始列下标。

   输入参数 `int __column2` ，表示区域查询的结束列下标。（闭区间）

2. 时间复杂度

   $O(\log(m\cdot n))$ 。
   
3. 备注

   本方法只有在做差函数合理的时候有意义。

#### 10.查询全部

1. 数据类型

2. 时间复杂度

   $O(\log(m\cdot n))$ 。

### 三、模板示例

```c++
#include "DS/BIT2d.h"
#include "IO/LeetcodeIO.h"

int main() {
    //二维树状数组与一维树状数组的用法基本类似，所以只展示一下初始化的特殊方法
    int matrix[4][5] = {
        {3, 0, 1, 4, 2},
        {5, 6, 3, 2, 1},
        {1, 2, 0, 1, 5},
        {4, 1, 0, 1, 7},
    };
    //除了行数、列数，还需要传递一个寻址函数
    OY::BIT2d_ex T([&](int i, int j) { return matrix[i][j]; }, 4, 5);
    //输出[0,2]行，[1,4]列的和
    cout << "sum(matrix[0~2][1~4])=" << T.query(0, 2, 1, 4) << endl;

    //*********************************************************************
    //二维树状数组加强版只能传二元寻址函数，而普通版可以传一元的

    //普通的 <单点修改，单点/区域查询> 二维数组
    OY::BIT2d query_tree([&](int i) { return matrix[i]; }, 4, 5);
    cout << "sum(matrix[0~2][1~4])=" << query_tree.query(0, 2, 1, 4) << endl;

    //*********************************************************************
    //再展示一下，如何利用二维差分构建 <单点/区域修改，单点查询> 二维数组
    //首先每行差分
    for (int i = 0; i < 4; i++) std::adjacent_difference(matrix[i], matrix[i] + 5, matrix[i]);
    //再每列差分
    for (int j = 0; j < 5; j++)
        for (int i = 3; i; i--) matrix[i][j] -= matrix[i - 1][j];
    //在此基础上建立二维树状数组
    OY::BIT2d modify_tree([&](int i) { return matrix[i]; }, 4, 5);
    //我们来看看每个点的前缀和
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 5; j++)
            cout << modify_tree.presum(i, j) << (j == 4 ? '\n' : ' ');
    //如何实现修改呢？可以在纸上算一下，会发现，一个子矩形自增一个常量，只会对四个点产生差分影响
    //对 [row1~row2],[col1~col2] 进行 k 的修改，受影响的点是：
    //  [row1,col1]+=k
    //  [row1,col2+1]-=k
    //  [row2+1,col1]-=k
    //  [row2+1,col2+1]+=k
    //假如我们想对 [1,2],[1,3] 进行修改
    modify_tree.add(1,1,100);
    modify_tree.add(1,4,-100);
    modify_tree.add(3,1,-100);
    modify_tree.add(3,4,100);
    //看看结果
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 5; j++)
            cout << modify_tree.presum(i, j) << (j == 4 ? '\n' : ' ');
}
```

```
#输出如下
sum(matrix[0~2][1~4])=27
sum(matrix[0~2][1~4])=27
3 0 1 4 2
5 6 3 2 1
1 2 0 1 5
4 1 0 1 7
3 0 1 4 2
5 106 103 102 1
1 102 100 101 5
4 1 0 1 7

```
