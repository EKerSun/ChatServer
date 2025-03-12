#include "group.hpp"

Group::Group(int id, std::string name, std::string desc)
    : id_{id}, name_{name}, desc_{desc} {}

void Group::SetID(int id) { this->id_ = id; }
void Group::SetName(std::string name) { this->name_ = name; }
void Group::SetDesc(std::string desc) { this->desc_ = desc; }

int Group::GetID() { return this->id_; }
std::string Group::GetName() { return this->name_; }
std::string Group::GetDesc() { return this->desc_; }
std::vector<GroupUser> &Group::GetUsers() { return this->users_; }