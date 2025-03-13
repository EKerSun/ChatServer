#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>

// User表的ORM类
class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "")
        : id_{id}, name_{name}, desc_{desc} {}

    void SetID(int id) { this->id_ = id; }
    void SetName(std::string name) { this->name_ = name; }
    void SetDesc(std::string desc) { this->desc_ = desc; }

    int GetID() { return this->id_; }
    std::string GetName() { return this->name_; }
    std::string GetDesc() { return this->desc_; }
    std::vector<GroupUser> &GetUsers() { return this->users_; }

private:
    int id_;
    std::string name_;
    std::string desc_;
    std::vector<GroupUser> users_;
};
#endif