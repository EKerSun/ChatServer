/**
 * 定义聊天服务所处理的消息类型
 */

#ifndef PUBLIC_H
#define PUBLIC_H

enum EnMsgType
{
    REG_MSG = 1, // 注册消息
    LOGIN_MSG,   // 登录消息
    QUIT_MSG,    // 退出消息

    ADD_FRIEND_MSG,    // 添加好友信息
    DELETE_FRIEND_MSG, // 删除好友信息
    CREATE_GROUP_MSG,  // 创建群组
    ADD_GROUP_MSG,        // 加入群组
    ONE_CHAT_MSG,      // 聊天消息
    GROUP_CHAT_MSG,    // 群聊天
    LOGOUT_MSG,        // 注销

    REG_MSG_ACK,        // 注册响应消息
    LOGIN_MSG_ACK,      // 登录响应消息
    ADD_FRIEND_MSG_ACK, // 添加好友响应
    DELETE_FRIEND_MSG_ACK, // 删除好友响应
    CREATE_GROUP_MSG_ACK, // 创建群组响应
    ADD_GROUP_MSG_ACK,    // 加入群组响应

};

#endif