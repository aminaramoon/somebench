#include "config.hpp"
#include <future>

#include <misc/timer.hpp>
#include "config.hpp"
#include "publisher.hpp"
#include "tcp_publisher.hpp"

int main() {
  tcp_publisher publisher;
  publisher.init();
  publisher.run(std::string(MESSAGE_SIZE, 'a'), std::chrono::milliseconds(DELAY_MS));
}
