#include "config.hpp"
#include <future>

#include <misc/timer.hpp>
#include "publisher.hpp"
#include "config.hpp"

int main()
{
    SomeIpNetworkPublisherNode publisher;

    auto w = std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        publisher.Exit();
    });

    publisher.Init(IS_TCP);
    const std::string payload(MESSAGE_SIZE, 'a');
    Timer timer(std::chrono::milliseconds(100), std::chrono::seconds(2));
    publisher.Execute(payload, timer);
}