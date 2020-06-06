
#include "config.hpp"
#include <thread>
#include <misc/timer.hpp>
#include "subscriber.hpp"

int main()
{
    SomeIpNetworkSubscriberNode subscriber;

    auto t = std::thread([&]() {
        std::cout << "running" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(9));
        subscriber.Exit();
    });

    subscriber.Init();
    subscriber.Execute();
    t.join();
}
