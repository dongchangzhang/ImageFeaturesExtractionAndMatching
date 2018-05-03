### 功能概览

语言：c++, 使用部分c++11语法

支持：sift、surf、kaze、akaze

基于opencv3.41 (使用sift和surf，opencv必须包含opencv_contrib)

所使用的特征匹配优化方法：

    1. 最佳匹配[distance] / 次佳匹配[distance] > 0.8
    2. ransac过滤
    3. 双向匹配过滤
> 其中距离筛选和ransac筛选是固有的，双向匹配可选。

### 文件概览
```
.
├── drawLine.cpp // 测试工具：绘制匹配线
├── drawPoints.cpp // 测试工具：绘制匹配点
├── feature.cpp // 特征提取匹配工具实现
├── feature.h // 特征提取匹配工具头文件
├── images // 测试图片
├── main.cpp 
├── Makefile
└── README.md

1 directory, 7 files

```

### 运行测试
编译

``` shell
cd ImageFeatureExtractionAndMatching
make
```

运行测试，查看各种特征的提取效果和匹配效果

```shell
./main image1 image2 ouput.txt
```

可以对输出结果绘制匹配效果
```shell
# 画点
./drawPoints image1 image2 ouput.txt
# 画线
./drawLine image1 image2 ouput.txt
```

### 接口介绍

> 使用该工具，只需要用到 feature.h 和feature.cpp两个文件

> 如果开启debug模式，需要查看运行过程中的输出：特征数目，绘制匹配结果等，请到feature.h中将#define _SHOW_IMAGE_AND_LOG_取消注释

```cpp
// api for feature match
std::vector<MatchedPoint> getMatchedPoints(
    const std::string &srcImage,
    const std::string &objImage,
    FeatureType type=SIFT,
    Direction direction=SINGLE);

```
功能：输入图片，根据需要的特征提取特征、进行匹配，然后对匹配结果进行筛选，并最终返回最佳匹配点对。

输入：待处理图片1、待处理图片2、特征类型（可选参数，默认为SIFT）、处理方向（可选参数，默认为单向，如果使用该参数，type也必须传递）。

返回：匹配点对组（MatchedPoint类型的vector）。

参数细节：
输入图片：c++字符串类型的图片位置，最好保证图片存在，虽然在程序中对不存在的情况已经进行了处理。

特征类型：有以下几种可用的类型：

    1. FeatureType::SITF
    2. FeatureType::SURF
    3. FeatureType::KAZE
    4. FeatureType::AKAZE

> 程序中harris脚点检测部分目前不可用

处理方向：支持两种方向

    1. Direction::BOTH
    2. Direction::SINGLE

> SINGLE: 使用img1和img2提取特征之后，
匹配过程只有match(img1, img2)

> BOTH: 匹配过程包括match(img1, img2); match(imag2, img1); 二者取交集。取交集过程使用hash方法。

> BOTH参数会增加处理时间

MatchedPoint类型为自定义类，可以到feature.h查看结构体细节。其内容包括图一的特征点，图二的特征点，二者的距离三部分信息。其中点使用opencv中的Point2f结构。

> 可以查看main.cpp查看例子。

### 匹配效果
对四种特征进行匹配，如下是不同图片进行匹配的结果，每一行分别为：图片1的特征点数、图片2的特征点数、使用距离过滤后匹配的点数、使用距离和ransac后匹配的点数，使用三种方法过滤的点数。

> 运行时间：kaze最慢，sift次之，akaze和surf较快

> 经过最后both的过滤得到的点很少，但是基本都能匹配上

1. images/2.JPG 和 images/3.JPG

| type     | sift | kaze | akaze | surf |
| -------- | ---- | ---- | ----- | ---- |
| image1   | 328  | 363  | 436   | 2547 |
| image2   | 373  | 276  | 375   | 3814 |
| distance | 63   | 58   | 34    | 216  |
| ransac   | 25   | 17   | 16    | 47   |
| both     | 17   | 4    | 7     | 15   |

2. images/2.JPG 和 images/4.JPG

> 虽然得到很多点，但是这次匹配效果最差，之后经过both过滤得到的点全是匹配的，其他含有不匹配点

| type     | sift | kaze | akaze | surf |
| -------- | ---- | ---- | ----- | ---- |
| image1   | 328  | 363  | 436   | 2547 |
| image2   | 208  | 263  | 356   | 1897 |
| distance | 44   | 28   | 34    | 149  |
| ransac   | 12   | 11   | 13    | 14   |
| both     | 4    | 0    | 5     | 1    |

3. images/5.JPG 和 images/6.JPG

| type     | sift | kaze | akaze | surf |
| -------- | ---- | ---- | ----- | ---- |
| img1     | 254  | 277  | 427   | 1212 |
| img2     | 506  | 551  | 715   | 2419 |
| distance | 52   | 50   | 121   | 183  |
| ransac   | 17   | 24   | 71    | 32   |
| both     | 3    | 5    | 35    | 9    |

4. images/9.JPG 和 images/10.JPG

| type   | sift  | kaze  | akaze | surf  |
| ------ | ----- | ----- | ----- | ----- |
| img1   | 17157 | 13496 | 10291 | 22308 |
| img2   | 16451 | 12697 | 9774  | 20883 |
| dist   | 1394  | 806   | 809   | 1378  |
| ransac | 1188  | 461   | 573   | 883   |
| both   | 926   | 285   | 338   | 492   |
