#include "friendmodel.hpp"
#include "db.hpp"

// 添加好友关系
bool FriendModel::Insert(int userid, int friendid)
{
    int max_id = std::max(userid, friendid);
    int min_id = std::min(userid, friendid);
    // 单向存储，较小id为id1,较大id为di2
    char sql[1024] = {0};
    sprintf(sql, " INSERT INTO friendships (user_id1, user_id2) VALUES (%d, %d)", min_id, max_id);
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}
// 删除好友
bool FriendModel::Delete(int userid, int friendid)
{
    int max_id = std::max(userid, friendid);
    int min_id = std::min(userid, friendid);
    char sql[1024] = {0};
    sprintf(sql, "DELETE FROM friendships WHERE user_id1 = %d AND user_id2 = %d", min_id, max_id);
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

// 返回用户好友列表
std::vector<User> FriendModel::Query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from user a inner join friendships b on case\
            when b.user_id1 = %d then b.user_id2 \
            when b.user_id2 = %d then b.user_id1 \
            end where b.user_id1 = %d or b.user_id2 = %d",
            userid, userid, userid, userid);
    std::vector<User> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.SetID(atoi(row[0]));
                user.SetName(row[1]);
                user.SetState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}