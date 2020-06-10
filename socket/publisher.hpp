#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>
#include <string>

#include "config.hpp"
#include "misc/message_fragmenter.hpp"
#include "misc/timer.hpp"

class udp_publisher {
 public:
  udp_publisher() {}

  ~udp_publisher() { exit(); }

  bool init() {
    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    cmd_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ == -1 || cmd_socket_ == -1) {
      std::cout << ">>>> error ||| "
                << "failed to initialize sockets" << std::endl;
      return false;
    }

    recipient_.sin_addr.s_addr = inet_addr(CLIENT_IP);
    recipient_.sin_family = AF_INET;
    recipient_.sin_port = htons(DATA_PORT);

    struct sockaddr_in command;
    command.sin_addr.s_addr = inet_addr(SERVER_IP);
    command.sin_family = AF_INET;
    command.sin_port = htons(CMD_PORT);

    const int enable = 1;
    int option = SO_REUSEADDR | SO_REUSEPORT;
    setsockopt(cmd_socket_, SOL_SOCKET, option, &enable, sizeof(int));
    if (bind(cmd_socket_, (const sockaddr *)&command, sizeof(command)) < 0) {
      std::cout << ">>>> error ||| "
                << "failed to bind to address" << std::endl;
      return false;
    }

    return true;
  }

  void send(const std::string &message, std::chrono::milliseconds spacing) {
    while (!is_ending_) {
      auto before = std::chrono::system_clock::now();
      std::this_thread::sleep_for(spacing);
      auto delay = std::chrono::system_clock::now() - before;
      delays_.emplace_back(std::chrono::duration_cast<std::chrono::microseconds>(delay));

      fragmenter_.Feed(message, false);
      auto payloads = fragmenter_.GetFragmentedMessages();
      mark_payloads(payloads);
      for (const auto &payload : payloads) {
        vsomeip::byte_t *data = payload->get_data();
        data += MultipartMessageHeaderSize;
        std::uint64_t current_ts_ = std::chrono::system_clock::now().time_since_epoch().count();
        std::memcpy((void *)data, (const void *)&current_ts_, sizeof(std::uint64_t));
        sendto(socket_, (const void *)payload->get_data(), payload->get_length(), 0,
               (const struct sockaddr *)&recipient_, sizeof(recipient_));
        n_packets_sent_++;
      }
      n_message_sent_++;
    }
  }

  void mark_payloads(std::vector<std::shared_ptr<vsomeip::payload>> &payloads) {
    std::int64_t current_ts = std::chrono::system_clock::now().time_since_epoch().count();
    for (const auto &payload : payloads) {
      vsomeip::byte_t *data = payload->get_data();
      data += MultipartMessageHeaderSize;
      std::memcpy((void *)data, (const void *)&current_ts, sizeof(std::int64_t));
    }
  }

  void run(const std::string &message, std::chrono::milliseconds spacing) {
    std::thread command_thread([this]() { on_command(); });

    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk, [this]() { return is_listening_.load(); });
    send(message, spacing);

    std::cout << ">>>> info ||| "
              << "sent # packets " << n_packets_sent_ << " # message " << n_message_sent_
              << " with spacing of " << spacing.count() << " millisecond " << std::endl;

    command_thread.join();
  }

  bool exit() {
    shutdown(socket_, SHUT_RDWR);
    shutdown(cmd_socket_, SHUT_RDWR);
    return true;
  }

  void on_command() {
    std::cout << ">>>> info ||| "
              << "publisher is ready for commands" << std::endl;
    int cmd = 0;

    struct sockaddr sender;
    socklen_t len;

    do {
      recvfrom(cmd_socket_, (void *)&cmd, sizeof(int), 0, &sender, &len);
      if (cmd == -124) {
        is_listening_ = true;
        cv_.notify_one();
        std::cout << ">>>> info ||| "
                  << "publisher is waiting for shutdown message" << std::endl;
        sendto(cmd_socket_, (const void *)&cmd, sizeof(int), 0, &sender, len);
      }
    } while (cmd < 0);

    last_id_ = cmd;

    is_ending_ = true;
    std::cout << ">>>> info ||| "
              << "shutdown message has been received" << std::endl;
    cv_.notify_one();
  }

 private:
  MessageFragmenter fragmenter_;
  int socket_ = -1, cmd_socket_ = -1;
  struct sockaddr_in recipient_;
  bool is_reliable_{false};
  std::atomic<bool> is_listening_{false};
  std::atomic<bool> is_ending_{false};
  std::size_t n_packets_sent_{0}, n_message_sent_{0};
  std::mutex mutex_;
  std::condition_variable cv_;
  std::vector<std::chrono::microseconds> delays_;
  int last_id_{0};
};
