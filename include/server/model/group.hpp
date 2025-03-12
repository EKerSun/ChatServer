#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>

// User表的ORM类
class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "");

    void SetID(int id);
    void SetName(std::string name);
    void SetDesc(std::string desc);

    int GetID();
    std::string GetName();
    std::string GetDesc();
    std::vector<GroupUser> &GetUsers();

private:
    int id_;
    std::string name_;
    std::string desc_;
    std::vector<GroupUser> users_;
};

#endif