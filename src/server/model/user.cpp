#include "user.hpp"

User::User(int id, std::string name, std::string password, std::string state)
    : id_(id), name_(name), password_(password), state_(state) {}
// 设置方法
void User::SetID(int id) { this->id_ = id; }
void User::SetName(std::string name) { this->name_ = name; }
void User::SetPassword(std::string password) { this->password_ = password; }
void User::SetState(std::string state) { this->state_ = state; }
// 查询方法
int User::GetId() { return id_; }
std::string User::GetName() { return name_; }
std::string User::GetPassword() { return password_; }
std::string User::GetState() { return state_; }