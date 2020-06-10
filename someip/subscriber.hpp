// Copyright Aeva 2020

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

#include <vsomeip/vsomeip.hpp>

#include "misc/message_reassembler.hpp"
#include "misc/timer.hpp"

class subscriber {
 public:
  enum SubscriptionReturnCode : std::uint8_t { ACCPETED = 0x00, REJECTED = 0x07 };

  subscriber() : application_(vsomeip::runtime::get()->create_application("client-sample")) {}

  ~subscriber() { exit(); }

  bool init() {
    if (!application_->init()) {
      std::cout << ">>>> error: ||| "
                << "Couldn't initialize application" << std::endl;
      return false;
    }

    application_->register_state_handler([this](vsomeip::state_type_e e) { this->on_state(e); });

    application_->register_message_handler(service_id_, instance_id_, event_id_,
                                           [this](const auto &pub_msg) { this->on_data(pub_msg); });

    application_->register_availability_handler(
        service_id_, instance_id_,
        [this](vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
          this->on_availability(service, instance, is_available);
        });

    application_->register_subscription_status_handler(
        service_id_, instance_id_, eventgroup_id_, event_id_,
        [this](const vsomeip::service_t service, const vsomeip::instance_t instance,
               const vsomeip::eventgroup_t eventgroup, const vsomeip::eventgroup_t event_id,
               const uint16_t sub_code) {
          this->on_subscription_status_change(service, instance, eventgroup, event_id, sub_code);
        });

    application_->register_subscription_handler(
        service_id_, instance_id_, eventgroup_id_,
        [this](const vsomeip::client_t client, const vsomeip::uid_t uid, const vsomeip::gid_t gid,
               const bool stats) -> bool {
          return on_subscription_change(client, uid, gid, stats);
        });

    application_->request_service(service_id_, instance_id_);

    std::set<vsomeip::eventgroup_t> its_groups;
    its_groups.insert(eventgroup_id_);
    application_->request_event(service_id_, instance_id_, event_id_, its_groups,
                                vsomeip::event_type_e::ET_EVENT);
    application_->subscribe(service_id_, instance_id_, eventgroup_id_);

    return true;
  }

  void start() {
    someip_thread_ = std::thread([this]() { application_->start(); });
    is_someip_running_ = true;
  }

  void run(const std::chrono::milliseconds &duration) {
    auto ready_thread = std::thread([this]() { inform_sender(); });
    auto shutdown_thread = std::thread([this, duration]() { close_sender(duration); });

    while (!stop_token_) {
      auto [reassembled_msg, id, latency] = reassembler_.GetReassembledMessage();
      latencies.emplace_back(latency);
      if (!reassembled_msg.empty() && !stop_token_)
        message_counter_++;
      else
        break;

      if (message_counter_ == 100U) {
        last_ts_ = std::chrono::system_clock::now();
        last_id_ = id;
      }
    }

    ready_thread.join();
    shutdown_thread.join();
  }

  bool exit() {
    stop_token_ = true;
    reassembler_.Notify();
    application_->clear_all_handler();
    application_->unsubscribe(service_id_, instance_id_, eventgroup_id_);
    application_->release_event(service_id_, instance_id_, event_id_);
    application_->release_service(service_id_, instance_id_);
    application_->stop();
    if (someip_thread_.joinable()) {
      someip_thread_.join();
      is_someip_running_ = false;
    }
    return true;
  }

  void on_state(vsomeip::state_type_e e) {
    is_registered_ = e == vsomeip::state_type_e::ST_REGISTERED;
    std::cout << ">>>> info ||| "
              << (is_registered_ ? "service is registered" : "service is de-registered")
              << std::endl;
    cv_.notify_one();
  }

  void on_data(const std::shared_ptr<vsomeip::message> &message) {
    static bool is_first = true;
    if (is_first) {
      first_ts_ = std::chrono::system_clock::now();
      std::cout << ">>>> info ||| "
                << "recording the first packet arrival timestamp" << std::endl;
      is_first = true;
    }
    reassembler_.Feed(message);
    packet_count_++;
  }

  void on_availability(vsomeip::service_t service, vsomeip::instance_t instance,
                       bool is_available) {
    is_available_ = service == service_id_ && instance_id_ == instance && is_available;
    std::cout << ">>>> info ||| "
              << "publisher service is " << (is_available_ ? "available" : "not availale")
              << std::endl;
  }

  void inform_sender() {
    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk, [this]() { return is_subscribed_.load(); });
    std::cout << ">>>> info ||| "
              << "sending ack message to publisher" << std::endl;
    std::shared_ptr<vsomeip::message> ready_req = vsomeip::runtime::get()->create_request();
    ready_req->set_service(service_id_);
    ready_req->set_instance(instance_id_);
    ready_req->set_method(ready_method_id_);
    application_->send(ready_req);

    start_clock_ = true;
    cv_.notify_one();
  }

  void close_sender(const std::chrono::milliseconds &duration) {
    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk, [this]() { return start_clock_.load(); });

    std::cout << ">>>> info ||| "
              << "starting the clock for end" << std::endl;

    std::this_thread::sleep_for(duration);

    std::cout << ">>>> info ||| "
              << "sending shutdown message to publisher" << std::endl;
    std::shared_ptr<vsomeip::message> ready_req = vsomeip::runtime::get()->create_request();
    ready_req->set_service(service_id_);
    ready_req->set_instance(instance_id_);
    ready_req->set_method(shutdown_method_id_);
    application_->send(ready_req);

    std::cout << ">>>> info ||| "
              << "# packets received " << packet_count_ << ", # messages received "
              << message_counter_ << std::endl;

    exit();
  }

  void on_subscription_status_change(const vsomeip::service_t service,
                                     const vsomeip::instance_t instance,
                                     const vsomeip::eventgroup_t eventgroup,
                                     const vsomeip::eventgroup_t event_id, const uint16_t subcode) {
    is_subscribed_ = service == service_id_ && instance == instance_id_ &&
                     eventgroup == eventgroup_id_ && event_id == event_id_ &&
                     subcode == SubscriptionReturnCode::ACCPETED;
    std::cout << ">>>> info ||| "
              << (is_subscribed_ ? "subscribed to eventgroup" : "not subscribed to eventgroup")
              << std::endl;
    cv_.notify_all();
  }

  bool on_subscription_change(const vsomeip::client_t client, const vsomeip::uid_t uid,
                              const vsomeip::gid_t gid, const bool stats) {
    std::cout << ">>>> info ||| " << (stats ? "client/uid/gid => true" : "client/uid/gid => false")
              << std::endl;
    return stats;
  }

 private:
  std::shared_ptr<vsomeip::application> application_;
  MessageReassembler reassembler_;
  std::uint16_t service_id_ = 0x1234, instance_id_ = 0x5678;
  std::uint16_t eventgroup_id_ = 0x4455, event_id_ = 0x8777;
  std::uint16_t ready_method_id_ = 0x7777;
  std::uint16_t shutdown_method_id_ = 0x8888;
  std::thread someip_thread_;
  std::atomic<bool> stop_token_{false};
  std::atomic<bool> is_registered_{false};
  std::atomic<bool> is_available_{false};
  std::atomic<bool> is_subscribed_{false};
  std::atomic<bool> is_someip_running_{false};
  std::atomic<bool> start_clock_{false};
  std::size_t message_counter_{0};
  std::size_t packet_count_{0};
  std::mutex mutex_;
  std::condition_variable cv_;
  std::chrono::system_clock::time_point first_ts_, last_ts_;
  std::vector<std::chrono::microseconds> latencies;
  int last_id_{0};
};
