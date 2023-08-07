# 基于物理的渲染（PBR）

## 一、简介

​		Physically-Based Rendering (PBR) 指的是一些在不同程度上都基于与现实世界的物理原理更相符的基本理论所构成的渲染技术的集合。由于基于物理的渲染旨在以物理上可信的方式模拟光线，因此它通常看起来比我们原始的光照算法，如Phong和Blinn-Phong更为真实。它不仅外观更佳，而且由于它紧密地近似真实的物理现象，艺术家可以基于物理参数创作表面材料，而不必依赖于便宜的技巧和调整来使光照看起来正确。基于物理参数创作材料的一个更大的优点是，这些材料在任何光照条件下都会看起来正确。

**核心原则：**

1. **能量守恒**
2. **基于微表面理论**
3. **使用基于物理的BRDF**

**主要组件：**

1. **Albedo / Base Color**：这是物体的基本颜色，不考虑任何光照或阴影。
2. **Metallic**：这描述了材料的金属性质。高值表示材料是金属，而低值表示它是非金属。
3. **Roughness**：描述了表面的粗糙程度。高值表示一个粗糙的表面，而低值表示一个光滑的表面。
4. **Normal Map**：用于添加高度的细节，使模型看起来更加复杂而不增加实际的多边形。
5. **Ambient Occlusion**：表示光线如何在微小的曲面细节中被遮挡。



## 二、文件说明

bin：存放.exe可执行文件、resources资源文件夹、.vs/.fs着色器文件。

src：存放源代码文件。



## 三、运行环境

操作系统：win10

运行、编码软件：Visual Studio 2022

C++标准：ISO C++14

外部库：GLFW、GLAD、GLM、Assimp、imgui



## 四、使用手册

​		如果您只需要使用，不需要更改程序源代码，您只需点击bin文件夹中的PBR.exe可执行文件，运行程序后，您可以在图形界面中看到渲染结果。

​		如果您需要更改程序源代码，您需要先确保已经配置好所需的外部库文件（GLFW、GLAD、GLM、Assimp、imgui），您可以通过Visual Studio打开并构建项目。

### 键盘事件

Esc：退出程序

W、A、S、D：分别控制摄像机向上、左、下、右移动。

N：显示并启用光标（修改参数时使用）

M：隐藏并禁用光标（查看渲染结果时使用）

### 其他设置

- Camera Speed：相机移动速度
- Mouse Sensitivity：鼠标灵敏度
- Light Color：光源颜色（由于采用了HDR，所以输入的颜色值允许大于255）
- Light Position：光源位置坐标（注：渲染窗口中的金色材质的小球代表光源）
- PokeBall Translate：PokeBall的位移矩阵
- PokeBall Scale：PokeBall的缩放矩阵
- Tank Translate：Tank的位移矩阵
- Tank Scale：Tank的缩放矩阵



## 五、结果展示

### 模型一：Tank

**渲染结果：**

![image-20230806173604167](/README.assets/image-20230806173604167.png)

**Sketchfab的渲染结果：**

URL：https://sketchfab.com/3d-models/kv-2-heavy-tank-1940-ba8b84d78c0a42038cf2eaa4210ef296

![image-20230806193344581](/README.assets/image-20230806193344581.png)

### 模型二：PokeBall

**渲染结果：**

![image-20230806174324328](/README.assets/image-20230806174324328.png)

**Sketchfab的渲染结果：**

URL：https://sketchfab.com/3d-models/pbr-pokeball-97336ec59f0146e8a48a93961dd0f502

![image-20230806173832644](/README.assets/image-20230806173832644.png)



## 六、对比分析

​		通过对比与Sketchfab的实现，我发现我的PBR算法在大部分情境下都能产生相似的渲染效果，这证明了我的实现是正确的。由于本次学习时间有限，某些细节尚未完善，尤其是阴影方面，在Tank模型中这一问题较为明显（如下图所示）。在接下来的学习和实践中，我计划深入研究阴影映射、点光源阴影等相关技术，进一步提高渲染的真实感。

**自己实现的PBR：**

![image-20230806194156444](/README.assets/image-20230806194156444.png)

**Sketchfab：**

![image-20230806194202714](/README.assets/image-20230806194202714.png)



## 七、其他信息

作者：VittorioDong

版本：1.0.0

时间：AUG 5 2023

