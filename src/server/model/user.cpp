#include "user.hpp"

User::User(int id, std::string name, std::string password, std::string state)
    : id_(id), name_(name), password_(password), state_(state) {}
// 设置方法
void User::setId(int id) { this->id_ = id; }
void User::setName(std::string name) { this->name_ = name; }
void User::setPassword(std::string password) { this->password_ = password; }
void User::setState(std::string state) { this->state_ = state; }
// 查询方法
int User::getId() { return id_; }
std::string User::getName() { return name_; }
std::string User::getPassword() { return password_; }
std::string User::getState() { return state_; }