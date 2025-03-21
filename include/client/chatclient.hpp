/**
 * 客户端封装类
 */
#ifndef CHATCLIENT_H
#define CHATCLIENT_H
class ChatClient
{
public:
    ChatClient(const char *ip, unsigned short port);
    ChatClient(const ChatClient &) = delete;
    ChatClient &operator=(const ChatClient &) = delete;
    ~ChatClient();
    void Start();

private:
    void onConnect();
    void onMessage();
    const char *ip_;
    unsigned short port_;
    int clientfd_;
};
#endif
