/**
 * 实现对好友表中插入、删除和查询操作
 */

#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>
#include "user.hpp"
class FriendModel
{
public:
    // 添加好友关系
    bool Insert(int userid, int friendid);
    // 删除好友关系
    bool Delete(int userid, int friendid);
    // 返回用户好友列表
    std::vector<User> Query(int userid);
private:
};
#endif