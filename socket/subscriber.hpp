// Copyright Aeva 2020

#pragma once

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <string>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "config.hpp"
#include "misc/timer.hpp"
#include <vsomeip/vsomeip.hpp>
#include "misc/message_reassembler.hpp"

class SocketSubscriberNode
{
public:
    SocketSubscriberNode() {}

    ~SocketSubscriberNode() { Exit(); }

    bool Init(bool is_reliable)
    {
        socket_ = socket(AF_INET, is_reliable ? SOCK_STREAM : SOCK_DGRAM, 0);
        if (socket_ == -1)
            return false;

        struct sockaddr_in server;
        server.sin_addr.s_addr = inet_addr(IP);
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);

        const int enable = 1;
        int option = SO_REUSEADDR | SO_REUSEPORT;
        setsockopt(socket_, SOL_SOCKET, option, &enable, sizeof(int));
        bool status = bind(socket_, (const struct sockaddr *)&server, sizeof(server)) >= 0;

        reader_thread_ = std::thread([this]() {
            this->ReadMessages();
        });

        return status;
    }

    void Execute()
    {
        std::cout << "received this much " << std::endl;
        while (!stop_token_)
        {
            std::cout << "received this much " << std::endl;
            auto reassembled_msg = reassembler_.GetReassembledMessage();
            if (reassembled_msg.empty() || stop_token_)
                return;
        }
    }

    bool Exit()
    {
        shutdown(socket_, SHUT_RDWR);
        reassembler_.Notify();
        stop_token_ = true;
        reader_thread_.join();
        return true;
    }

    void ReadMessages()
    {
        bool keep_reading = true;
        std::cout << "received this much " << std::endl;
        while (keep_reading)
        {
            std::vector<std::uint8_t> buffer(1400);
            int bytes_recv = recvfrom(socket_, (void *)buffer.data(), 1400, 0, NULL, NULL);
            std::cout << "received this much " << bytes_recv << std::endl;
            keep_reading = bytes_recv > 0;
            if (bytes_recv)
            {
                auto message = vsomeip::runtime::get()->create_message(false);
                buffer.resize(bytes_recv);
                message->get_payload()->set_data(std::move(buffer));
                reassembler_.Feed(message);
            }
        }
    }

private:
    int socket_ = -1;
    bool is_reliable_{false};
    std::thread reader_thread_;
    MessageReassembler reassembler_;
    std::atomic<bool> stop_token_{false};
};
