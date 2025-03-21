#include "chatclient.hpp"
#include "clientservice.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <string.h>
#include <thread>

ChatClient::ChatClient(const char *ip, uint16_t port)
    : ip_(ip), port_(port) {}
ChatClient::~ChatClient()
{
    if (clientfd_ != -1)
        close(clientfd_);
}
void ChatClient::Start()
{
    // 创建client端的socket
    clientfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd_)
    {
        std::cerr << "socket create error" << std::endl;
        exit(-1);
    }
    onConnect();
    ClientService::Instance()->Init();
    std::thread read_tesk(std::bind(&ChatClient::onMessage, this));
    read_tesk.detach();
    ClientService::Instance()->LoginMenu(clientfd_);
}
void ChatClient::onConnect()
{
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port_);
    server.sin_addr.s_addr = inet_addr(ip_);
    // client和server进行连接
    if (-1 == connect(clientfd_, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        std::cerr << "connect server error" << std::endl;
        close(clientfd_);
        exit(-1);
    }
}
// 接收线程
void ChatClient::onMessage()
{
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd_, buffer, 1024, 0); // 阻塞了
        if (-1 == len || 0 == len)
        {
            close(clientfd_);
            exit(-1);
        }
        nlohmann::json js = nlohmann::json::parse(buffer);
        int msgid = js["msgid"].get<int>();
        auto msg_handler = ClientService::Instance()->GetReadTaskHandler(msgid); // 获取处理函数
        msg_handler(js);
    }
}
