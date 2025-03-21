
#include "clientservice.hpp"
#include <iostream>
#include "public.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

std::string GetCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}

ClientService::ClientService()
{
    // 登录页面业务处理
    sand_handler_map_.insert({LOGIN_MSG, std::bind(&ClientService::Login, this, std::placeholders::_1)});
    sand_handler_map_.insert({REG_MSG, std::bind(&ClientService::Register, this, std::placeholders::_1)});
    sand_handler_map_.insert({QUIT_MSG, std::bind(&ClientService::Quit, this, std::placeholders::_1)});
    // 主页面业务处理
    sand_handler_map_.insert({ADD_FRIEND_MSG, std::bind(&ClientService::AddFriend, this, std::placeholders::_1)});
    sand_handler_map_.insert({DELETE_FRIEND_MSG, std::bind(&ClientService::DeleteFriend, this, std::placeholders::_1)});
    sand_handler_map_.insert({ONE_CHAT_MSG, std::bind(&ClientService::FriendChat, this, std::placeholders::_1)});
    sand_handler_map_.insert({CREATE_GROUP_MSG, std::bind(&ClientService::CreateGroup, this, std::placeholders::_1)});
    sand_handler_map_.insert({ADD_GROUP_MSG, std::bind(&ClientService::AddGroup, this, std::placeholders::_1)});
    sand_handler_map_.insert({GROUP_CHAT_MSG, std::bind(&ClientService::GroupChat, this, std::placeholders::_1)});
    sand_handler_map_.insert({LOGOUT_MSG, std::bind(&ClientService::Logout, this, std::placeholders::_1)});
    // 响应业务处理
    read_handler_map_.insert({LOGIN_MSG_ACK, std::bind(&ClientService::LoginResponse, this, std::placeholders::_1)});
    read_handler_map_.insert({REG_MSG_ACK, std::bind(&ClientService::RegisterResponse, this, std::placeholders::_1)});
    read_handler_map_.insert({ADD_FRIEND_MSG_ACK, std::bind(&ClientService::AddFriendResponse, this, std::placeholders::_1)});
    read_handler_map_.insert({DELETE_FRIEND_MSG_ACK, std::bind(&ClientService::DeleteFriendResponse, this, std::placeholders::_1)});
    read_handler_map_.insert({ONE_CHAT_MSG, std::bind(&ClientService::FriendChatResponse, this, std::placeholders::_1)});
    read_handler_map_.insert({CREATE_GROUP_MSG_ACK, std::bind(&ClientService::CreateGroupResponse, this, std::placeholders::_1)});
    read_handler_map_.insert({ADD_GROUP_MSG_ACK, std::bind(&ClientService::AddGroupResponse, this, std::placeholders::_1)});
    read_handler_map_.insert({GROUP_CHAT_MSG, std::bind(&ClientService::GroupChatResponse, this, std::placeholders::_1)});
}

ClientService::~ClientService()
{
    if (sem_initialized_)
        sem_destroy(&rwsem_);
}

void ClientService::Init()
{
    sem_init(&rwsem_, 0, 0);
    sem_initialized_ = true;
}
ClientService *ClientService::Instance()
{
    static ClientService service;
    return &service;
}
void ClientService::LoginMenu(int clientfd)
{
    std::cout << "***************** LoginPage *****************" << std::endl;
    std::cout << REG_MSG << ".Register." << std::endl;
    std::cout << LOGIN_MSG << ".Login." << std::endl;
    std::cout << QUIT_MSG << ".Quit." << std::endl;
    std::cout << "*********************************************" << std::endl;
    std::string choice;
    std::cin >> choice;
    auto it = sand_handler_map_.find(atoi(choice.c_str()));
    if (it == sand_handler_map_.end())
    {
        std::cerr << "Wrong Number, please input again!" << std::endl;
        LoginMenu(clientfd);
    }
    it->second(clientfd);
}

void ClientService::MainMenu(int clientfd)
{
    while (is_main_menu_running)
    {
        std::cout << "***************** MainPage *****************" << std::endl;
        std::cout << ADD_FRIEND_MSG << ". AddFriend" << std::endl;
        std::cout << DELETE_FRIEND_MSG << ". DeleteFriend" << std::endl;
        std::cout << CREATE_GROUP_MSG << ". CreateGroup" << std::endl;
        std::cout << ADD_GROUP_MSG << ". AddGroup" << std::endl;
        std::cout << ONE_CHAT_MSG << ". FriendChat" << std::endl;
        std::cout << GROUP_CHAT_MSG << ". GroupChat" << std::endl;
        std::cout << LOGOUT_MSG << ". Logout" << std::endl;
        std::cout << "*********************************************" << std::endl;
        std::string choice;
        std::cin >> choice;
        auto it = sand_handler_map_.find(atoi(choice.c_str()));
        if (it == sand_handler_map_.end())
        {
            std::cerr << "Wrong Number, please input again!" << std::endl;
            MainMenu(clientfd);
        }
        it->second(clientfd); // 执行对应的处理函数
    }
    LoginMenu(clientfd);
}

// 获取消息对应的处理器
SendTaskHandler ClientService::GetSendTaskHandler(int msgid)
{
    auto it = sand_handler_map_.find(msgid);
    if (it == sand_handler_map_.end())
        return [=](int)
        { std::cout << "Can not find handler!" << std::endl; };
    else
        return sand_handler_map_[msgid];
}

ReadTaskHandler ClientService::GetReadTaskHandler(int msgid)
{
    auto it = read_handler_map_.find(msgid);
    if (it == read_handler_map_.end())
        return [=](int)
        { std::cout << "Can not find handler!" << std::endl; };
    else
        return read_handler_map_[msgid];
}

void ClientService::Login(int clientfd)
{
    while (true)
    {
        std::string userID;
        std::cout << "Please input the userID or input q to back to login menu: ";
        std::cin >> userID;
        if (userID == "q")
            break;
        std::string pwd;
        std::cout << "Please input your password:";
        std::cin >> pwd;

        nlohmann::json js;
        js["msgid"] = LOGIN_MSG;
        js["id"] = atoi(userID.c_str());
        js["password"] = pwd;
        std::string request = js.dump();

        // 发送登录请求消息
        int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
        if (len == -1)
        {
            std::cerr << "Send login msg error:" << std::endl;
            continue;
        }
        sem_wait(&rwsem_); // 等待信号量，由子线程处理完登录的响应消息后，通知这里
        if (is_login_success_)
        {
            // 进入聊天主菜单页面
            is_main_menu_running = true;
            MainMenu(clientfd);
        }
    }
}
void ClientService::Register(int clientfd)
{
    while (true)
    {
        std::string name;
        std::cout << "Please input the username or input q to back to login menu: ";
        std::cin >> name;
        if (name == "q")
            break;
        std::string pwd_first;
        std::string pwd_second;
        std::cout << "Please input your password:";
        std::cin >> pwd_first;
        std::cout << "Please confirm your password:";
        std::cin >> pwd_second;
        if (pwd_first != pwd_second)
        {
            std::cout << "The two passwords are inconsistent, please re-enter!" << std::endl;
            continue;
        }
        nlohmann::json js;
        js["msgid"] = REG_MSG;
        js["name"] = name;
        js["password"] = pwd_second;
        std::string request = js.dump();
        int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
        if (len == -1)
        {
            std::cerr << "Send reg msg error:" << request << std::endl;
            Register(clientfd);
        }
        sem_wait(&rwsem_); // 等待信号量，子线程处理完注册消息会通知
        LoginMenu(clientfd);
    }
}

void ClientService::Quit(int clientfd)
{
    close(clientfd);
    sem_initialized_ = false;
    sem_destroy(&rwsem_);
    exit(0);
}

void ClientService::AddFriend(int clientfd)
{
    while (true)
    {
        nlohmann::json js;
        std::string friendID;
        std::cout << "Please input the friend's ID or input q to back to main menu: ";
        std::cin >> friendID;
        if (friendID == "q")
            break;
        if (!current_user_friend_id_set_.count(atoi(friendID.c_str())))
        {
            std::cout << "Friend already exists!" << std::endl;
            continue;
        }
        js["msgid"] = ADD_FRIEND_MSG;
        js["id"] = current_user_.GetID();

        js["friendid"] = atoi(friendID.c_str());
        std::string buffer = js.dump();

        int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
        if (-1 == len)
        {
            std::cerr << "Send AddFriend message error" << std::endl;
            continue;
        }
        sem_wait(&rwsem_);
    }
}
void ClientService::DeleteFriend(int clientfd)
{
    while (true)
    {
        nlohmann::json js;
        std::string friendID;
        std::cout << "Please input the friend's ID or input q to back to main menu: ";
        std::cin >> friendID;
        if (friendID == "q")
            break;
        if (!current_user_friend_id_set_.count(atoi(friendID.c_str())))
        {
            std::cout << "Friend not found!" << std::endl;
            continue;
        }
        js["msgid"] = DELETE_FRIEND_MSG;
        js["id"] = current_user_.GetID();
        js["friendid"] = atoi(friendID.c_str());
        std::string buffer = js.dump();

        int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
        if (-1 == len)
        {
            std::cerr << "Send DeleteFriend message error" << std::endl;
            continue;
        }
        sem_wait(&rwsem_);
    }
}
void ClientService::FriendChat(int clientfd)
{
    while (true)
    {
        std::cout << "Please input the friend's ID or input q to back to main menu: ";
        std::string friendID;
        std::cin >> friendID;
        if (friendID == "q")
            break;
        if (!current_user_friend_id_set_.count(atoi(friendID.c_str())))
        {
            std::cout << "Friend not found!" << std::endl;
            continue;
        }
        nlohmann::json js;
        js["msgid"] = ONE_CHAT_MSG;
        js["id"] = current_user_.GetID();
        js["name"] = current_user_.GetName();
        js["toid"] = atoi(friendID.c_str());
        while (true)
        {
            std::string messages;
            std::cout << "Please input the messsge or input q to chat with other friend: ";
            std::cin >> messages;
            if (messages == "q")
                break;
            js["time"] = GetCurrentTime();
            js["msg"] = messages;
            std::string buffer = js.dump();
            int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
            if (-1 == len)
            {
                std::cerr << "Send FriendChat message error" << std::endl;
                continue;
            }
        }
    }
}
void ClientService::CreateGroup(int clientfd)
{
    while (true)
    {
        std::cout << "Please input the group name or input q to back to main menu: ";
        std::string groupname;
        std::cin >> groupname;
        if (groupname == "q")
            break;
        std::cout << "Please input the group description or input q to  back to main menu: ";
        std::string groupdesc;
        std::cin >> groupdesc;
        if (groupdesc == "q")
            break;
        nlohmann::json js;
        js["msgid"] = CREATE_GROUP_MSG;
        js["id"] = current_user_.GetID();
        js["groupname"] = groupname;
        js["groupdesc"] = groupdesc;
        std::string buffer = js.dump();
        int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
        if (-1 == len)
        {
            std::cerr << "Send CreateGroup message error" << std::endl;
            continue;
        }
        sem_wait(&rwsem_);
    }
}
void ClientService::AddGroup(int clientfd)
{
    while (true)
    {
        std::cout << "Please input the groupID or input q to back to main menu: ";
        std::string groupID;
        std::cin >> groupID;
        if (groupID == "q")
            break;
        if (!current_user_group_id_set_.count(atoi(groupID.c_str())))
        {
            std::cerr << "You are already in this group" << std::endl;
            continue;
        }
        nlohmann::json js;
        js["msgid"] = ADD_GROUP_MSG;
        js["id"] = current_user_.GetID();
        js["groupid"] = atoi(groupID.c_str());
        std::string buffer = js.dump();

        int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
        if (-1 == len)
        {
            std::cerr << "Send AddGroup message error" << std::endl;
            continue;
        }
        sem_wait(&rwsem_);
    }
}
void ClientService::GroupChat(int clientfd)
{
    while (true)
    {
        std::cout << "Please input the groupID or input q to back to main menu: ";
        std::string groupID;
        std::cin >> groupID;
        if (groupID == "q")
            break;
        if (!current_user_group_id_set_.count(atoi(groupID.c_str())))
        {
            std::cerr << "You are not in this group" << std::endl;
            continue;
        }
        nlohmann::json js;
        js["msgid"] = GROUP_CHAT_MSG;
        js["id"] = current_user_.GetID();
        js["name"] = current_user_.GetName();
        js["groupid"] = atoi(groupID.c_str());
        while (true)
        {
            std::string message;
            std::cout << "Please input the messsge or input q to chat with other group: ";
            std::cin >> message;
            if (message == "q")
                break;
            js["msg"] = message;
            js["time"] = GetCurrentTime();
            std::string buffer = js.dump();
            int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
            if (-1 == len)
            {
                std::cerr << "Send GroupChat message error" << std::endl;
                continue;
            }
        }
    }
}
void ClientService::Logout(int clientfd)
{
    nlohmann::json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = current_user_.GetID();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
        std::cerr << "Send logout msg error:" << std::endl;
    else
        is_main_menu_running = false;
}
void ClientService::ShowCurrentUserData()
{
    std::cout << "====================== Login Infomation ======================" << std::endl;
    std::cout << "  Current login userID: " << current_user_.GetID() << std::endl;
    std::cout << "  Current login username: " << current_user_.GetName() << std::endl;
    std::cout << "----------------------Friend List---------------------" << std::endl;
    if (!current_user_friend_list_.empty())
    {
        for (User &user : current_user_friend_list_)
            std::cout << "  User ID: " << user.GetID()
                      << ", Username: " << user.GetName()
                      << ", State: " << user.GetState()
                      << std::endl;
    }
    std::cout << "----------------------Group List----------------------" << std::endl;
    if (!current_user_group_list_.empty())
    {
        for (Group &group : current_user_group_list_)
        {
            std::cout << "  Group ID: " << group.GetID()
                      << ", Group Name: " << group.GetName()
                      << ", Group Description: " << group.GetDesc()
                      << std::endl;
            std::cout << "  Group Users:" << std::endl;
            for (GroupUser &user : group.GetUsers())
            {
                std::cout << "    User ID: " << user.GetID()
                          << ", Username: " << user.GetName()
                          << ", State: " << user.GetState()
                          << std::endl;
            }
        }
    }
    std::cout << "----------------------Offline Message----------------------" << std::endl;
    if (!current_user_offline_message_.empty())
    {
        for (std::string &message_info : current_user_offline_message_)
        {
            nlohmann::json message_js = nlohmann::json::parse(message_info);
            // time + [id] + name + " said: " + xxx
            if (ONE_CHAT_MSG == message_js["msgid"].get<int>())
            {
                std::cout << "  Time:" << message_js["time"].get<std::string>()
                          << " [ID: " << message_js["id"].get<int>()
                          << " name: " << message_js["name"].get<std::string>()
                          << "] said: " << message_js["msg"].get<std::string>() << std::endl;
            }
            else
            {
                std::cout << "  Group message of group :" << message_js["groupid"] << std::endl;
                std::cout << "    Time:" << message_js["time"].get<std::string>()
                          << " [ID: " << message_js["id"].get<int>()
                          << " name: " << message_js["name"].get<std::string>()
                          << "] said: " << message_js["msg"].get<std::string>() << std::endl;
            }
        }
    }
    std::cout << "======================================================" << std::endl;
}

// 登录页面响应
void ClientService::LoginResponse(nlohmann::json &js)
{
    if (0 != js["errno"].get<int>()) // 登录失败
    {
        std::cerr << js["errmsg"] << std::endl;
        is_login_success_ = false;
    }
    else // 登录成功
    {
        // 记录当前用户的id和name
        current_user_.SetID(js["id"].get<int>());
        current_user_.SetName(js["name"]);

        // 记录当前用户的好友列表信息
        if (js.contains("friends"))
        {
            // 初始化
            current_user_friend_list_.clear();
            current_user_friend_id_set_.clear();
            std::vector<std::string> friends = js["friends"];
            for (std::string &friend_info : friends)
            {
                nlohmann::json friend_js = nlohmann::json::parse(friend_info);
                User user;
                current_user_friend_id_set_.insert(friend_js["id"].get<int>());
                user.SetID(friend_js["id"].get<int>());
                user.SetName(friend_js["name"]);
                user.SetState(friend_js["state"]);
                current_user_friend_list_.push_back(user);
            }
        }
        // 记录当前用户的群组列表信息
        if (js.contains("groups"))
        {
            // 初始化
            current_user_group_list_.clear();
            std::vector<std::string> groups = js["groups"];
            for (std::string &group_info : groups)
            {
                nlohmann::json group_js = nlohmann::json::parse(group_info);
                Group group;
                current_user_group_id_set_.insert(group_js["id"].get<int>());
                group.SetID(group_js["id"].get<int>());
                group.SetName(group_js["groupname"]);
                group.SetDesc(group_js["groupdesc"]);

                std::vector<std::string> group_users = group_js["users"];
                for (std::string &user_info : group_users)
                {
                    GroupUser user;
                    nlohmann::json user_js = nlohmann::json::parse(user_info);
                    user.SetID(user_js["id"].get<int>());
                    user.SetName(user_js["name"]);
                    user.SetState(user_js["state"]);
                    user.SetRole(user_js["role"]);
                    group.GetUsers().push_back(user);
                }
                current_user_group_list_.push_back(group);
            }
        }
        // 前用户的离线消息  个人聊天信息或者群组消息
        if (js.contains("offlinemsg"))
            current_user_offline_message_ = js["offlinemsg"];
        // 显示登录用户的基本信息
        ShowCurrentUserData();
        is_login_success_ = true;
    }
    sem_post(&rwsem_); // 通知主线程
}
void ClientService::RegisterResponse(nlohmann::json &js)
{
    if (0 != js["errno"].get<int>()) // 注册失败
        std::cerr << "Name is already exist!" << std::endl;
    else // 注册成功
        std::cout << "Register success, userID is: " << js["id"]
                  << ", do not forget it!" << std::endl;
    sem_post(&rwsem_);
}

// 主页面响应
void ClientService::AddFriendResponse(nlohmann::json &js)
{
    if (0 != js["errno"].get<int>())
        std::cerr << "Add friend error!" << std::endl;
    else
    {
        std::cout << "Add friend success" << std::endl;
        current_user_friend_id_set_.insert(js["friendid"].get<int>());
    }
    sem_post(&rwsem_);
}
void ClientService::DeleteFriendResponse(nlohmann::json &js)
{
    if (0 != js["errno"].get<int>()) // 添加失败
        std::cerr << "Delete friend error." << std::endl;
    else
    {
        std::cout << "Delete group success." << std::endl;
        current_user_friend_id_set_.insert(js["friendid"].get<int>());
    }
    sem_post(&rwsem_); // 通知主线程
}
void ClientService::FriendChatResponse(nlohmann::json &js)
{
    std::cout << "Time:" << js["time"].get<std::string>()
              << " [ID: " << js["id"].get<int>()
              << " name: " << js["name"].get<std::string>()
              << "] said: " << js["msg"].get<std::string>() << std::endl;
}
void ClientService::CreateGroupResponse(nlohmann::json &js)
{
    if (0 != js["errno"].get<int>()) // 添加失败
        std::cerr << "Create group error!" << std::endl;
    else
    {
        current_user_group_id_set_.insert(js["groupid"].get<int>());
        std::cout << "Create group success" << std::endl;
    }
    sem_post(&rwsem_); // 通知主线程
}
void ClientService::AddGroupResponse(nlohmann::json &js)
{
    if (0 != js["errno"].get<int>()) // 添加失败
        std::cerr << "Add group error!" << std::endl;
    else
    {
        current_user_group_id_set_.insert(js["groupid"].get<int>());
        std::cout << "Add group success" << std::endl;
    }
    sem_post(&rwsem_); // 通知主线程
}
void ClientService::GroupChatResponse(nlohmann::json &js)
{
    std::cout << "Group message of group :" << js["groupid"] << std::endl;
    std::cout << "Time:" << js["time"].get<std::string>()
              << " [ID: " << js["id"].get<int>()
              << " name: " << js["name"].get<std::string>()
              << "] said: " << js["msg"].get<std::string>() << std::endl;
}
