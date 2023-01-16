# 图形学大作业

## 实现功能

- 基于硬件的路径追踪算法
- 增加一个传入参数：所有光源的idx，实现直接对光采样，效率迅速提升

- 理想镜面 principled 和理想折射面 transmissive
- 金属光泽材质 principled（Phong模型）

拓展功能：

- lambertian和Phong模型材料的cosine重要性抽样
- 利用微表面模型的btdf实现了自定义材质，并实现了基于brdf(btdf)的重要性抽样
- 完成了一个布料仿真程序
- 为场景中的一些物体贴上了Minecraft原版材质包（）
- 一个场景：房间

## 结果

![2023-01-15 (4)](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\2023-01-15 (4).png)

![image-20230115103802693](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\image-20230115103802693.png)

![2023-01-15 (2)](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\2023-01-15 (2).png)

![2023-01-15 (3)](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\2023-01-15 (3).png)

![2023-01-15 (12)](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\2023-01-15 (12).png)

桌子椅子来自于外部下载的模型

书架、床、柜子、墙用了MC的贴图

背景为base中的背景

床单由C++程序模拟生成

小球为基于微表面模型的btdf材质，大球为完美折射材质，桌子和窗框为principled材质，



## 还有待改进的地方

- btdf自定义材料有一定缺陷
- 本来想挂个窗帘，发现写透明窗帘材料有点麻烦就没写
- 床单材质为lambertian，不太符合布料的一些特征



 ## Credits

- naive path tracing 1.10
  - <https://zhuanlan.zhihu.com/p/475547095>
- principled material in Phong model BRDF 1.10
  - <https://zhuanlan.zhihu.com/p/500811555>
- cosine importance sampling
  - <https://zhuanlan.zhihu.com/p/360420413>
- directly sampling the light
  - <https://zhuanlan.zhihu.com/p/475547095>
- principled/transmissive material in microfacet model
  - <https://zhuanlan.zhihu.com/p/459557696>
  - <https://blog.uwa4d.com/archives/1582.html>
- texture from Minecraft

- microfacet model importance sampling
  - <https://zhuanlan.zhihu.com/p/360420413>
  - *Microfacet models for refraction through rough surfaces*