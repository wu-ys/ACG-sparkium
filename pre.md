# 图形学大作业

## 1 实现功能概要

基础功能：

- 基于硬件的路径追踪算法
- 通过硬件渲染器增加一个传入参数：所有光源的idx，实现直接对光采样，效率迅速提升

- 实现了理想镜面 specular 和理想折射面 transmissive，并可在Gui界面中直接调节材料的折射率。

- 利用Phong模型的BRDF，实现有光泽材质 principled（Phong模型）

拓展功能：

- 实现Lambertian和Phong模型材料的余弦重要性抽样
- 利用微表面模型实现了自定义透明材质，并实现了基于brdf(btdf)的重要性抽样
- 通过质点-弹簧模型，完成了一个布料仿真程序，并将布料放置在了场景内
- 为场景中的一些物体定义了uv坐标，贴上了（经过自己缝合的）Minecraft原版材质包
- 一个场景：房间

## 2 测试场景展示

![2023-01-15 (4)](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\2023-01-15 (4).png)

![image-20230115103802693](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\image-20230115103802693.png)

![2023-01-17 (5)](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\2023-01-17 (5).png)

![2023-01-15 (3)](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\2023-01-15 (3).png)











## 自定义场景展示

![2023-01-15 (12)](D:\wys\classfile\2022-2023-1\Graphics\project\sparkium\pre.assets\2023-01-15 (12).png)

图中桌子椅子来自于外部下载的模型，并使用Meshlab做了适当的简化；书架、床、柜子、墙为自己写的obj文件，定义了uv坐标，用了MC的材质贴图；

床单由单独的C++程序模拟生成，将每个点的位置写入obj文件并导入到场景中；

小球为基于微表面模型的btdf材质，大球为完美折射材质，桌子和窗框为principled光泽材质。



## 还有待改进的地方

- 用微表面模型只写了透射材质，没有写不透明材质的情况
- 本来想挂个窗帘，发现写透明窗帘材料有点麻烦就没写
- 床单材质直接写了lambertian，没有用专门的布料



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