
#pragma once

#include "segemented_msg.hpp"
#include <atomic>
#include <string>
#include <mutex>
#include <condition_variable>

struct MessageReassembler
{
    ~MessageReassembler() { Notify(); }

    void Feed(const std::shared_ptr<vsomeip::message> &message)
    {
        if (is_running_ && segmented_message_.Append(message) == SegmentedMessage::PacketStatus::NOTIFY)
        {
            has_message_ = true;
            cv_.notify_one();
            {
                std::unique_lock<std::mutex> lk{mutex_};
                cv_.wait(lk, [this]() { return !has_message_; });
            }
        }
    }

    std::string GetReassembledMessage()
    {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait(lk, [this]() { return has_message_ || !is_running_; });

        std::string msg;
        if (has_message_ && is_running_)
        {
            auto payload = segmented_message_.Data();
            msg = std::string{(const char *)payload->get_data(), payload->get_length()};
        }
        has_message_ = false;
        cv_.notify_one();
        return msg;
    }

    void Notify()
    {
        is_running_ = false;
        cv_.notify_all();
    }

private:
    SegmentedMessage segmented_message_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic_bool has_message_{false}, is_running_{true};
};