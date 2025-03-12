/**
 * 描述用户表的维护方法，涉及用户表的插入、查询、更新
 */
#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"
//user表的数据操作类
class UserModel
{
public:
    //User表的增加方法
    bool insert(User &user);
    //根据用户号码查询用户信息
    User query(int id);
    //更新用户的状态信息
    bool updateState(User user);
    //重置用户的状态信息
    void resetState();
};

#endif