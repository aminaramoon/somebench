
#include "config.hpp"
#include <thread>
#include <misc/timer.hpp>
#include "subscriber.hpp"

int main()
{
    subscriber subscriber;

    subscriber.init();
    subscriber.start();
    subscriber.run(std::chrono::seconds(2));

    std::this_thread::sleep_for(std::chrono::seconds(9));
}
