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

class SomeIpNetworkPublisherNode
{
public:
    SomeIpNetworkPublisherNode()
        : application_(vsomeip::runtime::get()->create_application("SomeIpNetworkPublisherNode")) {}

    ~SomeIpNetworkPublisherNode() { Exit(); }

    bool Init(bool is_reliable)
    {
        service_id_ = SERVICE_ID;
        instance_id_ = INSTANCE_ID;

        if (!is_registered_ && application_->init())
        {
            application_->register_state_handler([this](vsomeip::state_type_e e) {
                is_registered_ = (e == vsomeip::state_type_e::ST_REGISTERED);
                if (e == vsomeip::state_type_e::ST_REGISTERED)
                {
                    application_->offer_service(service_id_, instance_id_);
                }
            });
        }

        is_reliable_ = is_reliable;
        auto offer_reliablity = is_reliable_ ? vsomeip::reliability_type_e::RT_RELIABLE
                                             : vsomeip::reliability_type_e::RT_UNRELIABLE;

        event_group_id_ = EVENTGROUP_ID;
        event_id_ = EVENT_ID;

        std::set<vsomeip::eventgroup_t> its_groups;
        its_groups.insert(event_group_id_);
        application_->offer_event(service_id_, instance_id_, event_id_, its_groups,
                                  vsomeip::event_type_e::ET_FIELD, std::chrono::milliseconds::zero(),
                                  false, true, nullptr, offer_reliablity);

        if (!is_someip_running_)
        {
            someip_thread_ = std::thread([this]() { application_->start(); });
            is_someip_running_ = true;
        }

        return true;
    }

    bool Execute(const std::string &message, Timer &timer)
    {
        timer.Begin();
        while (timer.IsReady())
        {
            std::cout << "sending message" << std::endl;
            fragmenter_.Feed(message, is_reliable_);
            auto payloads = fragmenter_.GetFragmentedMessages();
            for (const auto &payload : payloads)
            {
                application_->notify(service_id_, instance_id_, event_id_, payload);
                std::cout << "sending message" << std::endl;
            }
        }
        return true;
    }

    bool Exit()
    {
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

private:
    std::shared_ptr<vsomeip::application> application_;
    MessageFragmenter fragmenter_;
    std::uint16_t service_id_ = -1, instance_id_ = -1;
    std::uint16_t event_group_id_ = -1, event_id_ = -1;
    std::thread someip_thread_;
    bool is_reliable_{true};
    bool is_registered_{false};
    bool is_someip_running_{false};
};