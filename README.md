# protocol parser
# 配置项目
- "C:\\Qt\\Qt5.14.2\\5.14.2\\msvc2017_64\\include",
- "C:/Qt/Qt5.14.2/5.14.2/msvc2017_64/include/QtCore",
- "C:/Qt/Qt5.14.2/5.14.2/msvc2017_64/include/QtBluetooth"

# 配置qt路径安装目录 qtconfig：：setdir C:\Qt\Qt5.14.2

# cmake path
https://blog.csdn.net/qq_59470001/article/details/130799429
https://blog.csdn.net/qq_44002418/article/details/131724795

### 生成编译文件：
- cmake ../ -G Ninja -DCMAKE_BUILD_TYPE=Release -DADS_VERSION=5.14.2
- cmake ../ -G Ninja -DCMAKE_BUILD_TYPE=Debug -DADS_VERSION=5.14.2
### 编译
- NINJA
### Debug 信息输出
- 修改CMakeLists.txt文件注释WIN32 (#WIN32)

# 问题记录
- 血压返回数据auto字节 高四位表示体位 低四节表示测量方式 通过时间数据观测应该是协议写反了 
- 测量数据错误ec字段 0表示正常，1 表示没有测量 2未知 4未知 目前只发现这四个返回值(1 2 4 都是测量不正常)
- 设置测量计划 设置固定模式 设置白天开始时间9:30 设置时间间隔10分钟 然后启动（设置五分钟后开始测量）
- 关于设置按键是否可用 发送命令后需要等10秒启用才会生效（亲测验证）20 禁用生效
- 设置新的测量计划能覆盖旧的计划
- 设置好计划后测量应该是自动测量 如果失败或者报错后应该还会再测一次这样就会是自动重测again

问朱萍萍问题：
1. auto 字节
2. 覆盖计划（设置好计划后 怎么清除计划）
3. 设置好了测量计划后，又设置特殊测量，那么特殊测量接结束后 之前设置的计划会继续吗 
4. 设置好计划为执行，启动一次5分钟后执行就按照计划执行了 设置好启动计划并未启动（手动点击了一次）或者启用5分钟后启动
5. 关于手动测量我这里从auto字节根本就体现不出来了 这个需要做什么设置吗
