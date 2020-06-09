
#include "config.hpp"
#include <thread>
#include <misc/timer.hpp>
#include "subscriber.hpp"
#include "tcp_subscriber.hpp"

int main(int nargs, char** args) {
  if (nargs < 3) {
    std::cout << "socket_pub tcp/udp 1024" << std::endl;
    return 1;
  }

  if (strcmp(args[1], "tcp") == 0) {
    tcp_subscriber subscriber;
    int msg_len = std::stoi(args[2]);
    subscriber.init(msg_len + MultipartMessageHeaderSize);
    subscriber.run();
  } else {
    udp_subscriber subscriber;
    int msg_len = std::stoi(args[2]);
    subscriber.init();
    subscriber.run();
  }
}