/**
 * 基于muduo网络库的聊天服务的Server,提供基础的网络服务
 */

#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <string>

class ChatServer
{
public:
    // 初始化服务器
    ChatServer(muduo::net::EventLoop *loop,
               const muduo::net::InetAddress &listenAddr,
               const std::string &nameArg);
    // 启动服务器
    void Start();

private:
    // 上报链接相关信息的回调函数
    void onConnection(const muduo::net::TcpConnectionPtr &conn);
    // 上报读写事件相关的回调函数
    void onMessage(const muduo::net::TcpConnectionPtr &conn,
                   muduo::net::Buffer *buffer,
                   muduo::Timestamp time);
    muduo::net::TcpServer server_; // muduo库服务器类对象
    muduo::net::EventLoop *loop_;
};

#endif