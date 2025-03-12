#include "chatservice.hpp"
#include "public.hpp"
#include <functional>
#include <muduo/base/Logging.h>

ChatService::ChatService()
{
    // 给定消息类型id和对应的业务处理方法
    msg_handler_map_.insert({LOGIN_MSG, std::bind(&ChatService::login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msg_handler_map_.insert({REG_MSG, std::bind(&ChatService::reg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
}

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}
// 处理登录业务
void ChatService::login(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)
{
    /**
     * 业务描述：
     * js包括"id"，和"password"两项，分别表示账号和密码
     * 如果账号不存在，返回错误信息
     * 如果账号存在，但密码不正确，返回密码错误 | 密码正确但已经登录，返回已经登录
     * 账号密码正确，登录成功
     */
    LOG_INFO << "do login business!";
    int id = js["id"].get<int>();
    std::string password = js["password"];
    User user = usermodel_.query(id);
}
// 处理注册业务
void ChatService::reg(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)
{
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 利用map容器的特性，如果消息id不存在，返回默认处理器
    auto it = msg_handler_map_.find(msgid);
    if (it == msg_handler_map_.end())
    {
        // 返回默认处理器
        return [=](const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";
        };
    }
    else
    {
        return msg_handler_map_[msgid];
    }
}
