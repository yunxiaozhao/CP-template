### 一、模板类别

​	数据结构：二维差分。

### 二、模板功能

#### 1.初始化

1. 数据类型

   模板参数 `typename _Tp` ，表示元素类型。

   构造参数 `uint32_t __m`  ，表示维护的行范围为 `[0, m-1]` 。

   构造参数 `uint32_t __n`  ，表示维护的行范围为 `[0, n-1]` 。

2. 时间复杂度

   $O(m\cdot n)$ 。

3. 备注

   本数据结构，处于三种状态之一，且可以随时切换。

   当处于差分态 `AD2D_ADJACENT` 时，便于进行“将某个子矩形增加一定的值”操作；

   当处于值态时 `AD2D_VALUE` ，便于进行“将某个点增加一定的值”或者“查询某点的值”操作；

   当处于前缀和态 `AD2D_PRESUM` 时，便于进行“查询某个子矩形的和”操作。

   状态切换需要 $O(m\cdot n)$ 的时间，所以使用时尽量避免频繁切换状态。

   初始化后，由于每个点的值都是 `0` ，所以可以认为是任何一种状态，

#### 2.单点增值

1. 数据类型

   输入参数 `uint32_t __i`​ ，表示要增值的点所在行。

   输入参数 `uint32_t __j`​ ，表示要增值的点所在列。

   输入参数 `_Tp __inc`​ ，表示要增加的值。

2. 时间复杂度

   $O(1)$ 。

3. 备注

   本操作可以在差分态和值态两种状态下进行。如果此时状态不符，则会切换到值态。

   本函数没有进行参数检查，所以请自己确保下标合法。（行号、列号位于 `[0，m)`, `[0, n)`）

#### 3.区域增值

1. 数据类型

   输入参数 `uint32_t __r1`​ ，表示要增值的子矩形的开始行。

   输入参数 `uint32_t __r2`​ ，表示要增值的子矩形的结束行。

   输入参数 `uint32_t __c1`​ ，表示要增值的子矩形的开始列。

   输入参数 `uint32_t __c2`​ ，表示要增值的子矩形的结束列。

   输入参数 `_Tp __inc`​ ，表示要增加的值。

2. 时间复杂度

   $O(1)$ 。

3. 备注

   本操作可以在差分态下进行。如果此时状态不符，则会切换到差分态。

   本函数没有进行参数检查，所以请自己确保下标合法。（行号、列号位于 `[0，m)`, `[0, n)`）

#### 4.单点查询

1. 数据类型

   输入参数 `uint32_t __i`​ ，表示要增值的点所在行。

   输入参数 `uint32_t __j`​ ，表示要增值的点所在列。

2. 时间复杂度

   $O(1)$ 。

3. 备注

   本操作可以在值态下进行。如果此时状态不符，则会切换到值态。

   本函数没有进行参数检查，所以请自己确保下标合法。（行号、列号位于 `[0，m)`, `[0, n)`）

#### 5.区域查询

1. 数据类型

   输入参数 `uint32_t __r1`​ ，表示要查询的子矩形的开始行。

   输入参数 `uint32_t __r2`​ ，表示要查询的子矩形的结束行。

   输入参数 `uint32_t __c1`​ ，表示要查询的子矩形的开始列。

   输入参数 `uint32_t __c2`​ ，表示要查询的子矩形的结束列。

2. 时间复杂度

   $O(1)$ 。

3. 备注

   本操作可以在前缀和态下进行。如果此时状态不符，则会切换到前缀和态。

   本函数没有进行参数检查，所以请自己确保下标合法。（行号、列号位于 `[0，m)`, `[0, n)`）

#### 6.切换状态

1. 数据类型

   输入参数 `AD2D_STATE __state` ，表示要切换为的状态。

2. 时间复杂度

   $O(m\cdot n)$ 。

3. 备注

   一般情况下，状态会自动根据调用的函数切换，所以无需手动切换。

#### 7.全局差分

1. 数据类型

2. 时间复杂度

   $O(m\cdot n)$ 。

3. 备注

   在值态进行差分操作，可以切换到差分态。

   在前缀和态进行差分操作，可以切换到值态。

#### 8.全局前缀和

1. 数据类型

2. 时间复杂度

   $O(m\cdot n)$ 。

3. 备注

   在差分态进行前缀和操作，可以切换到值态。

   在值态进行前缀和操作，可以切换到前缀和态。


### 三、模板示例

```c++
#include "DS/AdjacentDifference2d.h"
#include "IO/FastIO.h"

int main() {
    OY::AdjacentDifference2d<int> ad(4, 5);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 5; j++)
            cout << ad.query(i, j) << (j == 4 ? '\n' : ' ');
    cout << '\n';

    ad.add(1, 2, 10);
    ad.add(2, 4, 100);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 5; j++)
            cout << ad.query(i, j) << (j == 4 ? '\n' : ' ');
    cout << '\n';

    ad.add(1, 3, 0, 2, 1000);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 5; j++)
            cout << ad.query(i, j) << (j == 4 ? '\n' : ' ');
    cout << '\n';
}
```

```
#输出如下
0 0 0 0 0
0 0 0 0 0
0 0 0 0 0
0 0 0 0 0

0 0 0 0 0
0 0 10 0 0
0 0 0 0 100
0 0 0 0 0

0 0 0 0 0
1000 1000 1010 0 0
1000 1000 1000 0 100
1000 1000 1000 0 0

```

