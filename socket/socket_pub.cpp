#include "config.hpp"
#include <future>
#include <cstring>
#include <string>

#include <misc/timer.hpp>
#include "config.hpp"
#include "publisher.hpp"
#include "tcp_publisher.hpp"

int main(int nargs, char** args) {
  if (nargs < 3) {
    std::cout << "socket_pub tcp/udp 1024" << std::endl;
    return 1;
  }

  if (strcmp(args[1], "tcp") == 0) {
    tcp_publisher publisher;
    int msg_len = std::stoi(args[2]);
    publisher.init();
    publisher.run(std::string(msg_len, 'a'), std::chrono::milliseconds(DELAY_MS));
  } else {
    udp_publisher publisher;
    int msg_len = std::stoi(args[2]);
    publisher.init();
    publisher.run(std::string(msg_len, 'a'), std::chrono::milliseconds(DELAY_MS));
  }
}
