# ChatServer
基于muduo网络库的集群式聊天服务器
## 功能
- 基于muduo网络库，利用IO复用技术Epoll与线程池实现多线程的Reactor高并发TCP服务端；
- 使用Json对通信请求和响应进行序列化和反序列化；
- 配置nginx基于TCP的负载均衡，实现聊天服务器的集群功能，提高后端服务的并发能力
- 基于redis的发布订-阅功能，实现跨服务器的消息通信
- 使用mysql关系型数据库作为项目数据的落地存储
- 高度解耦的网络模块和业务模块，实现基础的聊天业务，如登录、注册、群组、好友等业务。
## 环境要求
- Ubuntu-24.04
- C++17
## 依赖
- mysql
- redis
- nginx
- Json
## 目录树
```
.
├── CMakeLists.txt
├── bin                        # 可执行文件
│   ├── ChatClient
│   └── ChatServer
├── build                      # make目录          
│   └── Makefile
├── chat_database.sql          # nysql 脚本文件
├── include                    # 头文件
│   ├── public.hpp
│   └── server
│       ├── chatserver.hpp
│       ├── chatservice.hpp
│       ├── db
│       ├── model
│       └── redis
└── src                        # 源文件
    ├── CMakeLists.txt
    ├── client                 # 客户端
    │   ├── CMakeLists.txt
    │   └── main.cpp
    └── server                 # 服务端
        ├── CMakeLists.txt
        ├── chatserver.cpp
        ├── chatservice.cpp
        ├── db
        ├── main.cpp
        ├── model
        └── redis
```
## 项目启动
```mysql
# 创建数据库
CREATE DATABASE chat_database;
# 需要先修改src/server/db/db.cpp的user和password
# 使用数据库
USE chat_database;
# 执行脚本文件，如果有权限问题，需要chmod修改权限
source chat_database.sql；
```
```bash
# 修改nginx.conf配置文件
vi /etc/nginx/nginx.conf
# 或者使用提供的nginx.conf替换之
mv nginx.conf /etc/nginx/nginx.conf
# 运行服务端
# 需先确保mysql、redis和nginx服务启动
cd bin
# 在多个终端分别运行多个服务器，测试负载均衡
./ChatServer 127.0.0.1 6000
./ChatServer 127.0.0.1 6002
...

# 运行客户端，8000是nginx监听的窗口
./ChatClient 127.0.0.1 8000
```
## 说明
本项目是个人学习项目，参考施磊老师的集群聊天服务器








