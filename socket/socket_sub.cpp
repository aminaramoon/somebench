
#include "config.hpp"
#include <thread>
#include <misc/timer.hpp>
#include "subscriber.hpp"

int main()
{
    SocketSubscriberNode subscriber;

    auto t = std::thread([&]() {
        std::cout << "running" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        subscriber.Exit();
    });

    subscriber.Init(false);
    subscriber.Execute();
    t.join();
}