#include "config.hpp"
#include <future>

#include <misc/timer.hpp>
#include "publisher.hpp"

int main()
{
    SocketPublisherNode publisher;

    auto w = std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        publisher.Exit();
    });

    publisher.Init(false);
    const std::string payload(MESSAGE_SIZE, 'a');
    Timer timer(std::chrono::milliseconds(100), std::chrono::seconds(2));
    publisher.Execute(payload, timer);
}