/**
 * 描述用户的类对象
 */

#ifndef USER_H
#define USER_H
#include <string>

class User
{
public:
    User(int id = -1, std::string name = "", std::string password = "", std::string state = "offline")
        : id_{id}, name_{name}, password_{password}, state_{state} {}
    // 设置方法
    void SetID(int id) { this->id_ = id; }
    void SetName(std::string name) { this->name_ = name; }
    void SetPassword(std::string password) { this->password_ = password; }
    void SetState(std::string state) { this->state_ = state; }
    // 查询方法
    int GetID() { return id_; }
    std::string GetName() { return name_; }
    std::string GetPassword() { return password_; }
    std::string GetState() { return state_; }

private:
    int id_;
    std::string password_;
    std::string name_;
    std::string state_;
};
#endif
