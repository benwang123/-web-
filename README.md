基于游双《高性能服务器编程》源码进行重新开发，添加和修改了如下功能

1 利用配置文件传递服务器的相关参数，方便进行个性化配置

2 利用最小堆来实现定时器的管理，及时处理死链

3 利用redis和mysql来实现数据库的配置，使用连接池实现

4 支持reactor与proactor两种并发模型

5 利用内存池来实现内存管理
