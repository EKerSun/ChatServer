#include "chatserver.hpp"
#include <iostream>
#include "chatservice.hpp"

int main()
{
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChatServer");
    server.Start();
    loop.loop();
    return 0;
}
