#pragma once

#include <future>
#include <string>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"
#include "boost/format.hpp"
#include <chrono>

typedef std::chrono::system_clock Clock;
typedef std::chrono::time_point<Clock> TimePoint;
typedef TimePoint::duration Duration;

template<class Logger>
static std::future<void> startIoService(boost::asio::io_service& service, Logger& logger, const std::string& key)
{
    return std::async([&service, &logger, key]()
    {
        try
        {
            service.run();
        }
        catch(boost::system::system_error& e)
        {
            logger.errorStream() << boost::format("%1%: I/O service error: %2%") % key % e.code().message();
        }
        catch(std::exception& e)
        {
            logger.errorStream() << boost::format("%1%: I/O service error: %2%") % key % e.what();
            throw;
        }
        catch(...)
        {
            logger.errorStream() << "I/O service error: unknown error";
            throw;
        }
    });
}

template<class Logger>
void spawn(boost::asio::io_service& service, std::function<void(boost::asio::yield_context yield_)> worker, Logger& logger, std::string const& key)
{
    boost::asio::spawn(service, [worker, &logger, key](boost::asio::yield_context yield_)
    {
        try
        {
            worker(yield_);
        }
        catch(boost::system::system_error e)
        {
            logger.errorStream() << boost::format("%1%: thread id: %2%, error: %3%") % key % std::this_thread::get_id() % e.code().message();
        }
        catch(std::exception& e)
        {
            logger.errorStream() << boost::format("%1%: thread id: %2%, error: %3%") % key % std::this_thread::get_id() % e.what();
            throw;
        }
        catch(...)
        {
            logger.errorStream() << boost::format("%1%: thread id: %2%, error: unknown\n") % key % std::this_thread::get_id();
            throw;
        }
    });
}
