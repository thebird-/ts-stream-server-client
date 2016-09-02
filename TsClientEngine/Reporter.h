#pragma once

#include "boost/format.hpp"

#pragma warning(push)
#pragma warning(disable: 4706)
#include "log4cpp/Category.hh"
#pragma warning(pop)

#include "Misc.h"

class Reporter
{
public:
    Reporter(log4cpp::Category& logger)
        : m_logger(logger)
        , m_lastTime(Clock::now())
        , m_speedCounter(0)
        , m_total(0)
    {
    }

    ~Reporter()
    {
        if(m_speedCounter != 0)
        {
            doReport();
        }
        else if(m_total == 0)
        {
            m_logger.infoStream() << "Total received: 0 Kbytes";
        }
    }

    void OnReceived(size_t sent)
    {
        m_speedCounter += sent;
        m_total += sent;
        if(Clock::now() - m_lastTime > std::chrono::seconds(3))
        {
            doReport();
        }
    }

private:

    void doReport()
    {
        auto const sent = m_speedCounter;
        auto const duration = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - m_lastTime);
        auto const speed = 8.0 * sent / duration.count() * 1000 / 1024 / 1024;
        m_logger.infoStream() << boost::format("Total received: %1% Kbytes, current speed: %2$.3f Mbps") % (m_total / 1024) % speed;
        m_lastTime = Clock::now();
        m_speedCounter = 0;
    }

private:
    log4cpp::Category& m_logger;
    TimePoint m_lastTime;
    size_t m_speedCounter;
    size_t m_total;
};
