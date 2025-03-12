/**
 * 处理聊天业务的具体方法，被ChatServer调用
 */

#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include "usermodel.hpp"
#include <unordered_map>
#include <muduo/net/TcpServer.h>
#include <nlohmann/json.hpp>

// 消息处理函数类，作用是解耦合网络模块和业务模块
using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)>;

class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();
    // 处理登录业务
    void login(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 处理注册业务
    void reg(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);

private:
    ChatService();
    // 存储消息id和其对应的业务处理方法
    std::unordered_map<int, MsgHandler> msg_handler_map_;
    // 数据操作类对象
    UserModel usermodel_;
};

#endif