// Copyright Aeva 2020

#pragma once

#include <atomic>
#include <condition_variable>
#include <csignal>
#include <future>
#include <iostream>
#include <mutex>
#include <cmath>
#include <string>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.hpp"
#include "misc/message_reassembler.hpp"
#include "misc/timer.hpp"
#include <vsomeip/vsomeip.hpp>

class udp_subscriber {
 public:
  udp_subscriber() {}

  ~udp_subscriber() { exit(); }

  bool init() {
    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    cmd_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ == -1 || cmd_socket_ == -1) {
      std::cout << ">>>> error ||| "
                << "failed to initialize sockets" << std::endl;
      return false;
    }

    struct sockaddr_in data_addr;
    data_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(DATA_PORT);

    const int enable = 1;
    int option = SO_REUSEADDR | SO_REUSEPORT;
    setsockopt(socket_, SOL_SOCKET, option, &enable, sizeof(int));
    if (bind(socket_, (const struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
      std::cout << ">>>> error ||| "
                << "failed to bind to socket" << std::endl;
      return false;
    }

    server_addr_.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(CMD_PORT);

    std::cout << ">>>> info ||| "
              << "initializtion succesfull" << std::endl;

    return true;
  }

  void run() {
    auto reader_thread_ = std::thread([this]() { receive_message(); });
    auto coordinator = std::thread([this]() { coordinate(); });

    while (!stop_token_) {
      auto [reassembled_msg, id, latency] = reassembler_.GetReassembledMessage();
      latencies.emplace_back(latency);
      if (reassembled_msg.empty() || stop_token_) break;
      number_of_message_++;
      if (number_of_message_ == 100) {
        last_ts_ = std::chrono::system_clock::now();
        last_id_ = id;
        is_done_ = true;
        cv_.notify_one();
      }
    }

    coordinator.join();
    reader_thread_.join();
  }

  bool exit() {
    shutdown(socket_, SHUT_RDWR);
    shutdown(cmd_socket_, SHUT_RDWR);
    reassembler_.Notify();
    stop_token_ = true;
    if (reader_thread_.joinable()) reader_thread_.join();
    return true;
  }

  void receive_message() {
    is_ready_ = true;
    cv_.notify_one();
    bool keep_reading = true;
    bool first_read = true;
    while (keep_reading && !stop_token_) {
      std::vector<std::uint8_t> buffer(1400);
      int bytes_recv = recvfrom(socket_, (void *)buffer.data(), 1400, 0, NULL, NULL);

      if (first_read) {
        first_read = false;
        first_ts_ = std::chrono::system_clock::now();
      }

      keep_reading = bytes_recv > 0;
      if (bytes_recv) {
        number_packet_++;
        auto message = vsomeip::runtime::get()->create_message(false);
        buffer.resize(bytes_recv);
        message->get_payload()->set_data(std::move(buffer));
        reassembler_.Feed(message);
      }
    }
  }

  void coordinate() {
    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk, [this]() { return is_ready_.load(); });

    std::atomic_bool acknowledged{false};
    auto f = std::async(std::launch::async, [this, &acknowledged]() {
      int ack;
      std::cout << ">>>> info ||| "
                << "waiting for acknowledgement!" << std::endl;
      if (recvfrom(cmd_socket_, (void *)&ack, sizeof(int), 0, NULL, NULL) > 0) acknowledged = true;
    });

    while (!acknowledged) {
      std::cout << ">>>> info ||| "
                << "sending ready message" << std::endl;
      int message = -124;
      sendto(cmd_socket_, (const void *)&message, sizeof(int), 0, (const sockaddr *)&server_addr_,
             sizeof(server_addr_));
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    f.get();

    cv_.wait(lk, [this]() { return is_done_.load(); });

    std::cout << ">>>> info ||| "
              << "sending shutdown message" << std::endl;
    int message = 666;
    sendto(cmd_socket_, (const void *)&message, sizeof(int), 0, (const sockaddr *)&server_addr_,
           sizeof(server_addr_));

    auto stats = calcualte_stats();

    std::cout << ">>>> info ||| " << last_id_ << " in "
              << std::chrono::duration_cast<std::chrono::microseconds>(last_ts_ - first_ts_).count()
              << " microseconds "
              << "# packets received " << number_packet_ << ", # messages received "
              << number_of_message_ << std::endl;

    exit();
  }

  std::pair<double, double> calcualte_stats() const {
    double sum = 0.0, mean, std_dev = 0.0;

    for (const auto &latency : latencies) {
      std::cout << latency.count() << std::endl;
      sum += latency.count();
    }

    mean = sum / latencies.size();

    std::cout << "mean = " << mean << std::endl;

    for (const auto &latency : latencies) std_dev += std::pow(latency.count() - mean, 2);

    std_dev = std::sqrt(std_dev / latencies.size());

    std::cout << "std_dev = " << std_dev << std::endl;

    return std::make_pair(std::move(sum), std::move(std_dev));
  }

 private:
  int cmd_socket_ = -1;
  int socket_ = -1;
  bool is_reliable_{false};
  struct sockaddr_in server_addr_;
  std::thread reader_thread_;
  MessageReassembler reassembler_;
  std::atomic<bool> stop_token_{false};
  std::atomic_bool is_ready_{false};
  std::atomic_bool is_done_{false};
  std::mutex mutex_;
  std::condition_variable cv_;
  std::size_t number_packet_{0}, number_of_message_{0};
  std::chrono::system_clock::time_point first_ts_, last_ts_;
  std::vector<std::chrono::microseconds> latencies;
  int last_id_{0};
};
