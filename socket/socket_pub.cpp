#include "config.hpp"
#include <future>

#include <misc/timer.hpp>
#include "publisher.hpp"
#include "tcp_publisher.hpp"

int main()
{
  tcp_publisher publisher;
  publisher.init();
  publisher.run(std::string(1024, 'a'), std::chrono::milliseconds(10));
}
