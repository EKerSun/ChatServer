/**
 * 处理聊天业务的具体方法，被ChatServer调用
 */

#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include "usermodel.hpp"
#include <unordered_map>
#include <muduo/net/TcpServer.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include "friendmodel.hpp"
#include "offlinemessagemodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

// 消息处理函数类，作用是解耦合网络模块和业务模块
using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)>;

class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *Instance();
    // 处理登录业务
    void Login(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 处理注册业务
    void Register(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 添加好友业务
    void AddFriend(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 删除好友业务
    void DeleteFriend(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 一对一聊天业务
    void OneChat(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 创建群组业务
    void CreateGroup(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 加入群组业务
    void AddGroup(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 群组聊天业务
    void GroupChat(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 处理注销业务
    void LoginOut(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp);
    // 处理客户端异常退出
    void ClientCloseException(const muduo::net::TcpConnectionPtr &conn);
    // 服务器异常，业务重置方法
    void Reset();

    // 获取消息对应的处理器
    MsgHandler GetHandler(int msgid);
    // 从redis消息队列中获取订阅的消息
    void HandleRedisSubscribeMessage(int, std::string);

private:
    ChatService();
    // 存储消息id和其对应的业务处理方法
    std::unordered_map<int, MsgHandler> msg_handler_map_;
    // 数据操作类对象
    UserModel usermodel_;
    // 存储在线用户的通信连接
    std::unordered_map<int, muduo::net::TcpConnectionPtr> user_connection_map_;
    // 互斥锁
    std::mutex connection_mutex_;
    // 存储好友信息的操作类对象
    FriendModel friend_model_;
    // 存储离线消息
    OfflineMsgModel offline_msg_model_;
    // 存储群组
    GroupModel group_model_;
    // redis操作对象
    Redis redis_;
};

#endif