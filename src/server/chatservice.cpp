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
    msg_handler_map_.insert({ONE_CHAT_MSG, std::bind(&ChatService::OneChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msg_handler_map_.insert({ADD_FRIEND_MSG, std::bind(&ChatService::AddFriend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    // 群组业务管理相关事件处理回调注册
    msg_handler_map_.insert({CREATE_GROUP_MSG, std::bind(&ChatService::CreateGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msg_handler_map_.insert({ADD_GROUP_MSG, std::bind(&ChatService::AddGroup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msg_handler_map_.insert({GROUP_CHAT_MSG, std::bind(&ChatService::GroupChat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    // 下线
    msg_handler_map_.insert({LOGINOUT_MSG, std::bind(&ChatService::LoginOut, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
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

    if (id == -1 || user.GetID() == -1)
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
                usermodel_.updateState(user);

                nlohmann::json response;
                response["msgid"] = LOGIN_MSG_ACK;
                response["errno"] = 0;
                response["id"] = id;
                response["name"] = user.GetName();

                // 查询该用户的好友信息并返回
                std::vector<User> userVec = friend_model_.Query(id);
                if (!userVec.empty())
                {
                    std::vector<std::string> friend_info;
                    for (User &user : userVec)
                    {
                        nlohmann::json user_info;
                        user_info["id"] = user.GetID();
                        user_info["name"] = user.GetName();
                        user_info["state"] = user.GetState();
                        friend_info.push_back(user_info.dump());
                    }
                    response["friends"] = friend_info;
                }
                // 查询该用户的离线消息并返回
                std::vector<std::string> msg_vec = offline_msg_model_.Query(id);
                if (!msg_vec.empty())
                {
                    response["offlinemsg"] = msg_vec;
                    // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                    offline_msg_model_.Remove(id);
                }
                // 查询该用户的群组信息并返回
                std::vector<Group> groupVec = group_model_.QueryGroups(id);
                if (!groupVec.empty())
                {
                    std::vector<std::string> groupInfoVec;
                    for (Group &group : groupVec)
                    {
                        nlohmann::json js;
                        js["id"] = group.GetID();
                        js["groupname"] = group.GetName();
                        js["groupdesc"] = group.GetDesc();
                        std::vector<std::string> groupUserVec;
                        for (GroupUser &user : group.GetUsers())
                        {
                            nlohmann::json js2;
                            js2["id"] = user.GetID();
                            js2["name"] = user.GetName();
                            js2["role"] = user.GetRole();
                            js2["state"] = user.GetState();
                            groupUserVec.push_back(js2.dump());
                        }
                        js["users"] = groupUserVec;
                        groupInfoVec.push_back(js.dump());
                    }
                    response["groups"] = groupInfoVec;
                }

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
        response["id"] = user.GetID();
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
    if (!friend_model_.Insert(userid, friendid))
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

    if (!friend_model_.Delete(userid, friendid))
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
void ChatService::OneChat(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)
{
    int toid = js["toid"].get<int>();

    {
        std::lock_guard<std::mutex> lock(connection_mutex_);
        auto it = user_connection_map_.find(toid);
        if (it != user_connection_map_.end())
        {
            // toid在线，转发消息   服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }
    // toid不在线，存储离线消息
    offline_msg_model_.Insert(toid, js.dump());
}

// 创建群组业务
void ChatService::CreateGroup(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)
{
    int userid = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];
    // 存储新创建的群组信息
    Group group(-1, name, desc);

    if (!group_model_.CreateGroup(group))
    {
        nlohmann::json response;
        response["msgid"] = CREATE_GROUP_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "create group error!";
        conn->send(response.dump());
        // 存储群组创建人信息
    }
    else
    {
        group_model_.AddGroup(userid, group.GetID(), "creator");
        nlohmann::json response;
        response["msgid"] = CREATE_GROUP_MSG_ACK;
        response["errno"] = 0;
        response["errmsg"] = "create group success!";
        conn->send(response.dump());
    }
}

// 加入群组业务
void ChatService::AddGroup(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    if (!group_model_.AddGroup(userid, groupid, "normal"))
    {
        nlohmann::json response;
        response["msgid"] = ADD_GROUP_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "add group error!";
        conn->send(response.dump());
    }
    else
    {
        nlohmann::json response;
        response["msgid"] = ADD_GROUP_MSG_ACK;
        response["errno"] = 0;
        response["errmsg"] = "add group success!";
        conn->send(response.dump());
    }
}

// 群组聊天业务
void ChatService::GroupChat(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    std::vector<int> userid_vec = group_model_.QueryGroupUsers(userid, groupid);
    std::lock_guard<std::mutex> lock(connection_mutex_);
    for (int id : userid_vec)
    {
        auto it = user_connection_map_.find(id);
        if (it != user_connection_map_.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 存储离线群消息
            offline_msg_model_.Insert(id, js.dump());
        }
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

void ChatService::LoginOut(const muduo::net::TcpConnectionPtr &conn, nlohmann::json &js, muduo::Timestamp)
{
    int userid = js["id"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connection_mutex_);
        auto it = user_connection_map_.find(userid);
        if (it != user_connection_map_.end())
        {
            user_connection_map_.erase(it);
        }
    }
    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    usermodel_.updateState(user);
}

void ChatService::Reset()
{
    // 把所有在线用户的状态设置为离线
    usermodel_.resetState();
}

// 处理客户端异常退出
void ChatService::ClientCloseException(const muduo::net::TcpConnectionPtr &conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lock(connection_mutex_);
        for (auto it = user_connection_map_.begin(); it != user_connection_map_.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从在线用户容器中删除用户
                user.SetID(it->first);

                user_connection_map_.erase(it);
                break;
            }
        }
    }
    // 更新用户的状态信息为离线
    user.SetState("offline");
    usermodel_.updateState(user);
}