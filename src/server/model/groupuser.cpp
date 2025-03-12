#include "groupuser.hpp"

void GroupUser::SetRole(std::string role) { this->role_ = role; }
std::string GroupUser::GetRole() { return this->role_; }