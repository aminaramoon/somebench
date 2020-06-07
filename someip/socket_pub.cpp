#include "config.hpp"
#include <future>

#include <misc/timer.hpp>
#include "publisher.hpp"
#include "config.hpp"

int main()
{
    publisher publisher;

    publisher.init();
    publisher.start();
    publisher.run();

    std::this_thread::sleep_for(std::chrono::seconds(10));
    publisher.exit();
}
