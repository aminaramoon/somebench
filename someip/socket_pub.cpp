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

    publisher.run(std::string(1024, 'c'), std::chrono::milliseconds(1));

    std::this_thread::sleep_for(std::chrono::seconds(10));
    publisher.exit();
}
