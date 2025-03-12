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
    void setId(int id);
    void setName(std::string name);
    void setPassword(std::string password);
    void setState(std::string state);
    // 查询方法
    int getId();
    std::string getName();
    std::string getPassword();
    std::string getState();

private:
    int id_;
    std::string password_;
    std::string name_;
    std::string state_;
};

#endif