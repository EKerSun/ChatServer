#ifndef CLIENTSERVICE_H
#define CLIENTSERVICE_H
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <semaphore.h>
#include "group.hpp"
#include "user.hpp"


using SendTaskHandler = std::function<void(int)>;
using ReadTaskHandler = std::function<void(nlohmann::json &)>;


class ClientService
{
public:
    ClientService();
    ~ClientService();

    void Init();
    static ClientService *Instance();
    void LoginMenu(int clientfd);
    void MainMenu(int clientfd);
    

    // 获取消息对应的处理器
    SendTaskHandler GetSendTaskHandler(int msgid);
    ReadTaskHandler GetReadTaskHandler(int msgid);

    // 登录页面业务
    void Login(int clientfd);
    void Quit(int clientfd);
    void Register(int clientfd);

    // 主页面业务
    void AddFriend(int clientfd);
    void DeleteFriend(int clientfd);
    void FriendChat(int clientfd);
    void CreateGroup(int clientfd);
    void AddGroup(int clientfd);
    void GroupChat(int clientfd);
    void Logout(int clientfd);
    void ShowCurrentUserData();

    // 登录页面响应业务
    void LoginResponse(nlohmann::json &js);
    void RegisterResponse(nlohmann::json &js);
    // 主页面响应任务

    void AddFriendResponse(nlohmann::json &js);
    void DeleteFriendResponse(nlohmann::json &js);
    void CreateGroupResponse(nlohmann::json &js);
    void FriendChatResponse(nlohmann::json &js);
    void GroupChatResponse(nlohmann::json &js);
    void AddGroupResponse(nlohmann::json &js);

private:
    std::unordered_map<int, SendTaskHandler> sand_handler_map_; // 存储发送消息的任务
    std::unordered_map<int, ReadTaskHandler> read_handler_map_;
    bool is_main_menu_running = false;
    bool is_login_success_ = false;
    // 记录当前系统登录的用户信息
    User current_user_;
    // 记录当前登录用户的好友列表信息
    std::vector<User> current_user_friend_list_;
    std::unordered_set<int> current_user_friend_id_set_;
    // 记录当前登录用户的群组列表信息
    std::vector<Group> current_user_group_list_;
    std::unordered_set<int> current_user_group_id_set_;
    // 记录当前登录用户的离线消息
    std::vector<std::string> current_user_offline_message_;
    sem_t rwsem_;
    bool sem_initialized_;
};

#endif
