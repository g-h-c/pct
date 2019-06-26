---
typora-root-url: ./
---

### 修改说明

在原版基础上，主要做了如下增强：

1. 增加对VC2008(及其兼容工程)的支持，工程后缀为.vcproj
2. 增加头文件包含次数统计，并增加命令行参数mostincluded指定过滤最小包含次数，0表示不过滤
3. 增加命令行参数force控制是否覆盖stdafx.h文件
4. 默认只输出系统头文件，也就是<>括起来的头文件，在命令行参数includeheader中增加了特殊设置，用于输出所有头文件，用法：--includeheader *
5. 将stdafx.h文件输出到源代码目录，而不是输出到工程文件所在目录
6. 自动生成stdafx.cpp文件



### 集成到MSVC中

可以通过MSVC的"工具->外部工具"功能集成到IDE中，这样使用起来更方便，选中某个工程后，点击"工具->提取头文件"即可输出stdafx.h和stdafx.cpp到此工程源代码目录。

![1561518892864](/README/1561518892864.png)

完整的参数字段参考：

```
--vcproj $(ProjectDir)$(ProjectFileName) --sysinclude C:\greentools\ro\VC2008\VC\include --sysinclude C:\greentools\ro\VC2008\VC\PlatformSDK\Include --configuration "Debug|Win32" --force -m 10
```

