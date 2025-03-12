/**
 *
 * 存储用户离线消息的类
 */

#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>

// 提供离线消息表的操作接口方法
class OfflineMsgModel
{
public:
    // 存储用户的离线消息
    void Insert(int userid, std::string msg);

    // 删除用户的离线消息
    void Remove(int userid);

    // 查询用户的离线消息
    std::vector<std::string> Query(int userid);
};

#endif