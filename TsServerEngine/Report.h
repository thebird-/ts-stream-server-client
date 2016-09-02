#pragma once

#include <atomic>
#include <string>

#include "boost/format.hpp"

#include "Misc.h"

class Report
{
public:
    Report(const Duration& duration_)
        : m_duration(duration_)
        , m_last(TimePoint::clock::now())
        , m_speedCounter(0)
        , m_total(0)
    {
    }

    void OnSent(int64_t sent)
    {
        m_speedCounter += sent;
        m_total += sent;
    }

    template<class Stream>
    void DoReport(Stream out, bool force)
    {
        if(!force && TimePoint::clock::now() - m_last < m_duration)
        {
            return;
        }
        auto const bytes = std::atomic_exchange(&m_speedCounter, 0);
        if(bytes == 0)
        {
            return;
        }
        auto const milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(TimePoint::clock::now() - m_last).count();
        auto const mbits = bytes / 1024.0 / 1024.0 * 8.0;
        m_last = TimePoint::clock::now();
        out << getLog(m_total, mbits / milliseconds * 1000.0);
    }

private:
    virtual std::string getLog(int64_t total, double speed) = 0;

private:
    Duration const m_duration;
    TimePoint m_last;
    std::atomic_int64_t m_speedCounter;
    std::atomic_int64_t m_total;
};

class ReportChannel
    : public Report
{
public:
    ReportChannel(Duration const& duration, std::string const& key)
        : Report(duration)
        , m_key(key)
    {
    }

private:
    virtual std::string getLog(int64_t total, double speed)
    {
        return (boost::format("%1%: sent: %2% KBytes, %3$.3f MBits/sec") % m_key % (total / 1024) % speed).str();
    }

private:
    std::string const& m_key;
};

class ReportTotal
    : public Report
{
public:
    ReportTotal(Duration const& duration, std::atomic_uint const& clientCount)
        : Report(duration)
        , m_clientCount(clientCount)
    {
    }

private:
    virtual std::string getLog(int64_t total, double speed)
    {
        return (boost::format("Total: clients: %1%: sent: %2% KBytes, %3$.3f MBits/sec") % m_clientCount % (total / 1024) % speed).str();
    }

private:
    std::atomic_uint const& m_clientCount;
};
