#pragma once

#include <sys/socket.h>
#include <arpa/inet.h>

#include <string>
#include <iostream>

#include "misc/message_fragmenter.hpp"
#include "misc/timer.hpp"
#include "config.hpp"

class SocketPublisherNode
{
public:
    SocketPublisherNode() {}

    ~SocketPublisherNode() { Exit(); }

    bool Init(bool is_reliable)
    {
        std::cout << "hello" << std::endl;
        socket_ = socket(AF_INET, is_reliable ? SOCK_STREAM : SOCK_DGRAM, 0);
        if (socket_ == -1)
            return false;

        std::cout << "hello" << std::endl;
        server_.sin_addr.s_addr = inet_addr(IP);
        server_.sin_family = AF_INET;
        server_.sin_port = htons(PORT);

        std::cout << "hello" << std::endl;
        if (is_reliable)
            connect(socket_, (const struct sockaddr *)&server_, sizeof(server_));

        return true;
    }

    bool Execute(const std::string &message, Timer &timer)
    {
        timer.Begin();
        while (timer.IsReady())
        {
            fragmenter_.Feed(message, is_reliable_);
            auto payloads = fragmenter_.GetFragmentedMessages();
            for (const auto &payload : payloads)
            {
                if (is_reliable_) {
                    send(socket_, reinterpret_cast<const void *>(payload->get_data()), payload->get_length(), 0);
                    std::cout << "TCP message sent" << std::endl;
                }
                else
                {
                    sendto(socket_, (const void *)payload->get_data(), payload->get_length(), 0, (const struct sockaddr *)&server_, sizeof(server_));
                    std::cout << "message sent" << std::endl;
                }
            }
        }
        return true;
    }

    bool Exit()
    {
        shutdown(socket_, SHUT_RDWR);
        return true;
    }

private:
    MessageFragmenter fragmenter_;
    int socket_ = -1;
    struct sockaddr_in server_;
    bool is_reliable_{false};
};