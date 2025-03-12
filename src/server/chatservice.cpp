#include "chatservice.hpp"
#include "public.hpp"
#include <functional>
#include <muduo/base/Logging.h>

ChatService::ChatService()
{
    // 给定消息类型id和对应的业务处理方法
    msg_handler_map_.insert({REG_MSG, std::bind(&ChatService::Register, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msg_handler_map_.insert({LOGIN_MSG, std::bind(&ChatService::Login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msg_handler_map_.insert({ADD_FRIEND_MSG, std::bind(&ChatService::AddFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msg_handler_map_.insert({DELETE_FRIEND_MSG, std::bind(&ChatService::DeleteFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
}

// 获取单例对象的接口函数
ChatService *ChatService::Instance()
{
    static ChatService service;
    return &service;
}

// 处理登录业务
void ChatService::Login(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp time)
{
    /**
     * 业务描述：
     * recv js包括"msgid"、"id"和"password"，分别表示任务类别、账号和密码
     * 如果账号不存在，返回错误信息
     * 如果账号存在，但密码不正确，返回密码错误 | 密码正确但已经登录，返回已经登录
     * 账号密码正确，登录成功
     */
    LOG_INFO << "do login business!";
    int id = js["id"].get<int>();
    std::string password = js["password"];
    User user = usermodel_.query(id);

    if (id == -1 || user.GetId() == -1)
    {
        // 用户ID不存在, 查询用户表时返回的用户ID为-1
        nlohmann::json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "This user is not registered!";
        conn->send(response.dump());
    }
    else // 用户ID存在
    {
        if (user.GetPassword() != password)
        {
            // 密码错误

            nlohmann::json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "User name or password is error!";
            conn->send(response.dump());
        }
        else // 密码正确
        {
            if (user.GetState() == "online")
            {
                // 该用户已经登录，不允许重复登录
                nlohmann::json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 3;
                response["errmsg"] = "User already login!";
                conn->send(response.dump());
            }
            else
            {
                /**
                 * 登录成功后的操作：
                 * 1.用户连接信息注册到连接map
                 * 2.更新用户状态
                 * 3.编辑响应消息
                 */

                // 登录成功，记录用户连接信息
                {
                    std::lock_guard<std::mutex> lock(connection_mutex_);
                    user_connection_map_.insert({id, conn});
                }
                // 登录成功，更新用户状态信息
                user.SetState("online");
                nlohmann::json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 0;
                response["errmsg"] = "Login success!";
                response["id"] = id;
                response["name"] = user.GetName();

                conn->send(response.dump());
            }
        }
    }
}

// 处理注册业务
void ChatService::Register(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp time)
{
    // 业务处理：向redis写入注册信息
    LOG_INFO << "Do reg business!";
    std::string name = js["name"];
    std::string password = js["password"];
    User user;
    user.SetName(name);
    user.SetPassword(password);
    bool state = usermodel_.insert(user);
    if (state)
    {
        // 注册成功
        nlohmann::json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["errmsg"] = "Register success!";
        response["id"] = user.GetId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        nlohmann::json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "Register failed!";
        conn->send(response.dump());
    }
}

// 添加好友业务 msgid id friendid
void ChatService::AddFriend(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp time)
{
    LOG_INFO << "Add friend business!";

    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息
    if (!friendModel_.Insert(userid, friendid))
    {
        nlohmann::json response;
        response["msgid"] = ADD_FRIEND_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "Already friends or wrong user!";
        conn->send(response.dump());
    }
    else
    {
        nlohmann::json response;
        response["msgid"] = ADD_FRIEND_MSG_ACK;
        response["errno"] = 0;
        response["errmsg"] = "Add friend success!";
        conn->send(response.dump());
    }
}
// 删除好友业务
void ChatService::DeleteFriend(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)
{
    LOG_INFO << "Add friend business!";
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    if (!friendModel_.Delete(userid, friendid))
    {
        nlohmann::json response;
        response["msgid"] = DELETE_FRIEND_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "wrong user!";
        conn->send(response.dump());
    }
    else
    {
        nlohmann::json response;
        response["msgid"] = DELETE_FRIEND_MSG_ACK;
        response["errno"] = 0;
        response["errmsg"] = "Delete friend success!";
        conn->send(response.dump());
    }
}

// 获取消息对应的处理器
MsgHandler ChatService::GetHandler(int msgid)
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
