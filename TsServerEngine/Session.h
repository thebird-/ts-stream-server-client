#pragma once

#include <memory>
#include <future>

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"
#include "boost/asio/system_timer.hpp"

#include "SessionManagerCallback.h"
#include "Misc.h"

class Session
{
public:
    Session(SessionManagerCallback& server, intptr_t token, std::unique_ptr<boost::asio::io_service>&& service, boost::asio::ip::tcp::socket&& socket);
    void Cancel();

private:
    void session(boost::asio::yield_context yield_);
    std::string readPath(boost::asio::yield_context yield_);
    void waitNextTick(boost::asio::system_timer& timer, TimePoint const& tick, boost::asio::yield_context yield_);
    void sendData(boost::asio::ip::tcp::socket& socket, std::vector<char> const& buffer, boost::asio::yield_context yield_);

private:
    SessionManagerCallback& m_server;
    intptr_t m_token;
    std::unique_ptr<boost::asio::io_service> m_service;
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::system_timer m_timer;
    std::future<void> m_serviceThread;
    boost::asio::io_service::work m_work;
};
