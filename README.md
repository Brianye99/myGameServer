# myGameServer

实现了多人在线对战游戏的服务器，服务器负责分发游戏数据、世界聊天、同步游戏进度。基于LInux原生的epoll模型，采用三层架构形成的并发框架。

![image]([E:\mariamdown\pic\bdeab8d34f5fe10b6be5e4706e5bcca5bd9efe5f.png](https://github.com/Brianye99/myGameServer/blob/master/image/xmind.png))

涉及的技术包括protobuf、redis、时间轮定时器等。使用了责任链、工厂方法、代理等设计模式。

**特性：**

- 玩家上下线数据同步（基于AOI算法进行位置更新）
- 玩家信息管理（基于Redis存储和获取数据）
- 服务器延时自动关闭（轮转定时器）
- 游戏房间管理（Redis存储房间信息）

测试截图：

![image]([E:\markdown\pic\a9bd9bd1939c3b988bfce74cf5b68ea0cf586e58.png](https://github.com/Brianye99/myGameServer/blob/master/image/gameview.png))

![image]([E:\markdown\pic\ae0f725d7fb7e1177e7a9a4b07d263b58d0ce0ba.png](https://github.com/Brianye99/myGameServer/blob/master/image/serverview.png))
