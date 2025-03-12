/**
 * 描述用户的类对象
 */

#ifndef USER_H
#define USER_H
#include <string>

class User
{
public:
    User(int id = -1, std::string name = "", std::string password = "", std::string state = "offline");
    // 设置方法
    void SetID(int id);
    void SetName(std::string name);
    void SetPassword(std::string password);
    void SetState(std::string state);
    // 查询方法
    int GetId();
    std::string GetName();
    std::string GetPassword();
    std::string GetState();

private:
    int id_;
    std::string password_;
    std::string name_;
    std::string state_;
};

#endif