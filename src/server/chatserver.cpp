#include "chatserver.hpp"
#include "chatservice.hpp"
#include <functional>
#include <muduo/base/Logging.h>
#include <nlohmann/json.hpp>

ChatServer::ChatServer(muduo::net::EventLoop *loop,
                       const muduo::net::InetAddress &listenAddr,
                       const std::string &nameArg)
    : server_(loop, listenAddr, nameArg), loop_{loop}
{
    // 设置链接回调
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    // 设置消息回调
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置线程数量
    server_.setThreadNum(4);
}

// 启动服务器
void ChatServer::Start()
{
    server_.start();
}

// 上报链接相关信息的回调函数
void ChatServer::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    // 客户端断开连接
    if (!conn->connected())
    {
        LOG_INFO << "The client: " << conn->peerAddress().toIp() << " : " << conn->peerAddress().port() << " disconnects";
        conn->shutdown();
    }
}
// 上报读写事件相关的回调函数
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr &conn,
                           muduo::net::Buffer *buffer,
                           muduo::Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();
    // 数据的反序列化
    nlohmann::json js = nlohmann::json::parse(buf);
    // 通过js["msgid"] 获取业务handler conn json time
    // 完全解耦网络模块和业务模块
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, time);
}