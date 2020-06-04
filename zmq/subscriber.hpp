#pragma once

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <string>
#include <thread>

#include "misc/message_fragmenter.hpp"
#include "misc/timer.hpp"
#include <zmq.hpp>

class ZmqNetworkSubscriberNode
{
public:
    ZmqNetworkSubscriberNode() : context_{zmq::context_t(1)}, socket_(context_, ZMQ_SUB)
    {
    }

    ~ZmqNetworkSubscriberNode() { Exit(); }

    bool Init(bool is_reliable)
    {
        socket_.setsockopt(ZMQ_SUBSCRIBE, "", 0);
        socket_.bind(URL);
        return true;
    }

    bool Execute(const std::string &message, Timer &timer)
    {
        return true;
    }

    bool Exit()
    {
        return true;
    }

private:
    zmq::context_t context_;
    zmq::socket_t socket_;
    MessageFragmenter fragmenter_;
    std::uint16_t port_ = -1;
    std::string ip_;
    bool is_reliable_{true};
};