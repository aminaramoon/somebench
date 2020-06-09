
#include "config.hpp"
#include <thread>
#include <misc/timer.hpp>
#include "subscriber.hpp"
#include "tcp_subscriber.hpp"

int main()
{
    tcp_subscriber subscriber;
    subscriber.init(MESSAGE_SIZE + MultipartMessageHeaderSize);
    subscriber.run();
}