## 说明

此处写了两个日志报警类：
- 其中nsimplelogger是简单日志器，用于临时记录日志和测试，主要针对的是标准C++的日志记录
    - 这个日志器的重点在于checkLogFile和记录方式
- NViewLog的类是纯Qt实现的日志记录类，其使用还需要满足两个条件
    - 1. 必须在main函数中调用qInstallMessageHandler(onViewLog);
    - 2. 必须在编译前在.pro文件中添加DEFINES += QT_MESSAGELOGCONTEXT

## 推荐使用Qt日志记录

当发生库报警时，普通的std::cout无法输出日志，QDebug则可以正常输出日志。
原因在于库报警产生时重定向了输出文件，而std::cout一般是标准输出文件所以被影响，qDebug则使用的stderr，不会被影响。（这里说的比较模糊，具体情况请自己查阅）
