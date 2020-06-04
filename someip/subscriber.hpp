// Copyright Aeva 2020

#pragma once

#include <mutex>
#include <atomic>
#include <string>
#include <thread>
#include <condition_variable>

#include <vsomeip/vsomeip.hpp>

#include "misc/timer.hpp"
#include "misc/message_reassembler.hpp"

class SomeIpNetworkSubscriberNode
{
public:
    SomeIpNetworkSubscriberNode()
        : application_(vsomeip::runtime::get()->create_application("SomeIpNetworkSubscriberNode")) {}

    ~SomeIpNetworkSubscriberNode() { Exit(); }

    bool Init()
    {
        service_id_ = SERVICE_ID;
        instance_id_ = INSTANCE_ID;

        if (!is_registered_ && application_->init())
        {
            application_->register_state_handler([this](vsomeip::state_type_e e) {
                is_registered_ = (e == vsomeip::state_type_e::ST_REGISTERED);
                if (e == vsomeip::state_type_e::ST_REGISTERED)
                {
                    application_->request_service(service_id_, instance_id_);
                }
            });
        }

        event_group_id_ = EVENTGROUP_ID;
        event_id_ = EVENT_ID;

        application_->register_message_handler(
            service_id_, instance_id_, event_id_,
            [this](const auto &pub_msg) { this->OnRawDataArrival(pub_msg); });

        application_->register_subscription_status_handler(
            service_id_, instance_id_, event_id_, event_id_,
            [this](const vsomeip::service_t service, const vsomeip::instance_t instance,
                   const vsomeip::eventgroup_t eventgroup, const vsomeip::eventgroup_t event_id,
                   const uint16_t sub_code) {
                this->OnSubscriptionStatusChange(service, instance, eventgroup, event_id, sub_code);
            });

        std::set<vsomeip::eventgroup_t> its_groups;
        its_groups.insert(event_group_id_);
        application_->request_event(service_id_, instance_id_, event_id_, its_groups,
                                    vsomeip::event_type_e::ET_FIELD);
        application_->subscribe(service_id_, instance_id_, event_group_id_);

        if (!is_someip_running_)
        {
            someip_thread_ = std::thread([this]() { this->application_->start(); });
            is_someip_running_ = true;
        }

        return true;
    }

    bool Execute()
    {
        while (!stop_token_)
        {
            auto reassembled_msg = reassembler_.GetReassembledMessage();
            if (reassembled_msg.empty() || stop_token_)
                return false;
        }
        return true;
    }

    bool Exit()
    {
        stop_token_ = true;
        reassembler_.Notify();
        application_->clear_all_handler();
        application_->unsubscribe(service_id_, instance_id_, event_group_id_);
        application_->release_event(service_id_, instance_id_, event_id_);
        application_->release_service(service_id_, instance_id_);
        application_->stop();
        if (someip_thread_.joinable())
        {
            someip_thread_.join();
            is_someip_running_ = false;
        }
        return true;
    }

protected:
    enum SubscriptionReturnCode : std::uint16_t
    {
        ACCPETED = 0x00,
        REJECTED = 0x07
    };

private:
    void OnRawDataArrival(const std::shared_ptr<vsomeip::message> &message)
    {
        reassembler_.Feed(message);
    }

    void OnServiceAvailable(vsomeip::service_t service, vsomeip::instance_t instance,
                            bool is_available)
    {
        is_available_ = service == service_id_ && instance_id_ == instance && is_available;
    }

    void OnSubscriptionStatusChange(const vsomeip::service_t service,
                                    const vsomeip::instance_t instance,
                                    const vsomeip::eventgroup_t eventgroup,
                                    const vsomeip::eventgroup_t event_id, const uint16_t subcode)
    {
        is_subscribed_ = service == service_id_ && instance == instance_id_ && eventgroup == event_group_id_ &&
                         event_id == event_id_ && subcode == SubscriptionReturnCode::ACCPETED;
    }

private:
    std::shared_ptr<vsomeip::application> application_;
    MessageReassembler reassembler_;
    std::atomic<bool> stop_token_{false};
    std::string configuration_message_;
    std::uint16_t service_id_ = -1, instance_id_ = -1;
    std::uint16_t event_group_id_ = -1, event_id_ = -1;
    std::thread someip_thread_;
    bool is_registered_{false};
    bool is_available_{false};
    bool is_subscribed_{false};
    bool is_someip_running_{false};
};
