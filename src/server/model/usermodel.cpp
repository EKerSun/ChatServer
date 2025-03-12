#include "usermodel.hpp"
#include "db.hpp"

//User表的增加方法
bool UserModel::insert(User &user)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
            user.GetName().c_str(), user.GetPassword().c_str(), user.GetState().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            user.SetID(mysql_insert_id(mysql.getConn()));
            return true;
        }
    }
    return false;
}

// 根据用户号码查询用户信息
User UserModel::query(int id)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.SetID(atoi(row[0]));
                user.SetName(row[1]);
                user.SetPassword(row[2]);
                user.SetState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

bool UserModel::updateState(User user)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d",
            user.GetState().c_str(), user.GetId());
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
void UserModel::resetState()
{
    char sql[1024] = "update user set state = 'offline'";
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}