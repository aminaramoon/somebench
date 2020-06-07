#pragma once

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <thread>

#include "misc/message_fragmenter.hpp"
#include "misc/timer.hpp"
#include <vsomeip/vsomeip.hpp>

class publisher
{
public:
    publisher()
        : application_(vsomeip::runtime::get()->create_application("service-sample")) {}

    ~publisher() { exit(); }

    bool init()
    {
        if (!application_->init())
        {
            std::cout << ">>>> error ||| "
                      << "application initializtion failed" << std::endl;
            return false;
        }

        application_->register_state_handler([this](vsomeip::state_type_e e) {
            this->on_state(e);
        });

        std::set<vsomeip::eventgroup_t> its_groups;
        its_groups.insert(eventgroup_id_);
        application_->offer_event(service_id_, instance_id_, event_id_, its_groups,
                                  vsomeip::event_type_e::ET_EVENT, std::chrono::milliseconds::zero(),
                                  false, true, nullptr, vsomeip::reliability_type_e::RT_UNRELIABLE);

        application_->register_message_handler(service_id_, instance_id_, ready_method_id_, [this](const auto &msg) { this->on_ready(msg); });

        application_->register_message_handler(service_id_, instance_id_, shutdown_method_id_, [this](const auto &msg) { this->on_shutdown(msg); });

        return true;
    }

    void start()
    {
        someip_thread_ = std::thread([this]() { application_->start(); });
        is_someip_running_ = true;
    }

    void run(const std::string &message, std::chrono::milliseconds spacing)
    {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait(lk, [this]() { return is_registered_.load(); });
        offer();
        cv_.wait(lk, [this]() { return is_listening_.load(); });
        send(message, spacing);
    }

    void offer()
    {
        std::cout << ">>>> info ||| "
                  << "offering services" << std::endl;
        application_->offer_service(service_id_, instance_id_);
    }

    void send(const std::string &message, std::chrono::milliseconds spacing)
    {
        while (!is_ending_)
        {
            std::this_thread::sleep_for(spacing);
            fragmenter_.Feed(message, false);
            auto payloads = fragmenter_.GetFragmentedMessages();
            for (const auto &payload : payloads)
            {
                application_->notify(service_id_, instance_id_, event_id_, payload);
                n_packets_sent_++;
            }
        }
    }

    bool exit()
    {

        std::cout << "number of packets sent " << n_packets_sent_ << std::endl;
        application_->clear_all_handler();
        application_->stop_offer_service(service_id_, instance_id_);
        application_->stop();
        if (someip_thread_.joinable())
        {
            someip_thread_.join();
            is_someip_running_ = false;
        }
        return true;
    }

    void on_state(vsomeip::state_type_e e)
    {
        is_registered_ = e == vsomeip::state_type_e::ST_REGISTERED;
        std::cout << ">>>> info ||| "
                  << "service is " << (is_registered_ ? "registered" : "de-registered") << std::endl;
        cv_.notify_one();
    }

    void on_ready(const std::shared_ptr<vsomeip::message> &msg)
    {
        std::cout << ">>>> info ||| "
                  << "subscriber is ready to get messages" << std::endl;
        is_listening_ = true;
        cv_.notify_one();
    }

    void on_shutdown(const std::shared_ptr<vsomeip::message> &msg)
    {
        std::cout << ">>>> info ||| "
                  << "shutdown message has been received" << std::endl;
        is_ending_ = true;
        cv_.notify_all();
        exit();
    }

private:
    std::shared_ptr<vsomeip::application> application_;
    MessageFragmenter fragmenter_;
    std::uint16_t service_id_ = 0x1234, instance_id_ = 0x5678;
    std::uint16_t eventgroup_id_ = 0x4455, event_id_ = 0x8777;
    std::uint16_t ready_method_id_ = 0x7777;
    std::uint16_t shutdown_method_id_ = 0x8888;
    std::thread someip_thread_;
    std::atomic<bool> is_reliable_{true};
    std::atomic<bool> is_registered_{false};
    std::atomic<bool> is_someip_running_{false};
    std::atomic<bool> is_listening_{false};
    std::atomic<bool> is_ending_{false};
    std::size_t n_packets_sent_{0};
    std::mutex mutex_;
    std::condition_variable cv_;
};
