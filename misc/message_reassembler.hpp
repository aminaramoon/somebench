
#pragma once

#include "segemented_msg.hpp"
#include <atomic>
#include <string>
#include <mutex>
#include <tuple>
#include <condition_variable>

struct MessageReassembler {
  using clock_t = std::chrono::system_clock;

  ~MessageReassembler() { Notify(); }

  void Feed(const std::shared_ptr<vsomeip::message> &message) {
    if (is_running_ &&
        segmented_message_.Append(message) == SegmentedMessage::PacketStatus::NOTIFY) {
      auto now = clock_t::now();
      auto data_ptr = reinterpret_cast<const std::int64_t *>(message->get_payload()->get_data());
      clock_t::duration b(data_ptr[1]);
      clock_t::time_point acquisition_timestamp(b);

      latency_ = std::chrono::duration_cast<std::chrono::microseconds>(now - acquisition_timestamp);

      has_message_ = true;
      cv_.notify_one();
      {
        std::unique_lock<std::mutex> lk{mutex_};
        cv_.wait(lk, [this]() { return !has_message_; });
      }
    }
  }

  std::tuple<std::string, int, std::chrono::microseconds> GetReassembledMessage() {
    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk, [this]() { return has_message_ || !is_running_; });

    std::string msg;
    if (has_message_ && is_running_) {
      auto payload = segmented_message_.Data();
      msg = std::string{(const char *)payload->get_data(), payload->get_length()};
    }
    has_message_ = false;
    cv_.notify_one();
    return std::make_tuple(std::move(msg), segmented_message_.SessionId(), latency_);
  }

  void Notify() {
    is_running_ = false;
    cv_.notify_all();
  }

  std::chrono::microseconds Latency() const noexcept { return latency_; }

 private:
  SegmentedMessage segmented_message_;
  std::chrono::microseconds latency_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic_bool has_message_{false}, is_running_{true};
};