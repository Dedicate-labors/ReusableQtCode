这些拉流和推流一般都够应对大部分情况了，但是面对Ascend的DVPP这类output是H.26x 裸流的混蛋，你有两种选择：
1. 一个是自己组装AvPacket进行发送（效率高，但开发难度大）
2. 匿名管道Popen + ffmpeg指令进行推流（推荐，简单，无门槛）

自己写的相关笔记：https://my.feishu.cn/wiki/DxuGwwbzQiNb0ukyuwTcb7QHn4c?fromScene=spaceOverview
