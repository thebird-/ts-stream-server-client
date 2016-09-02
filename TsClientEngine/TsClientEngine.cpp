#define TS_CLIENT_ENGINE_IMPLEMENTATION
#include "TsClientEngine/TsClientEngine.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#include "log4cpp/Category.hh"
#pragma warning(pop)

#include "boost/asio.hpp"
#include "boost/asio/spawn.hpp"

#include <iostream>
#include <fstream>
#include <thread>

#include "Misc.h"
#include "Reporter.h"

namespace
{
    class ClientImp
        : public tsClient::Client
    {
    public:
        ClientImp(std::string const& address, short port, std::string const& fileName_, std::ofstream& destination, log4cpp::Category& logger)
            : m_logger(logger)
            , m_destination(destination)
            , m_socket(m_service)
            , m_shutdown(false)
        {
            std::string const fileName(fileName_ + std::string(1, '\0'));
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);
            spawn(m_service, [this, endpoint, fileName](boost::asio::yield_context yield_)
            {
                session(fileName, endpoint, yield_);
            }, m_logger, "Client");
            m_serviceThread = startIoService(m_service, m_logger, "Client");
        }

        virtual ~ClientImp()
        {
            m_service.post([this]()
            {
                cleanup();
            });
            m_serviceThread.wait();
        }

    private:

        void cleanup()
        {
            m_shutdown = true;
            m_socket.close();
        }

        void session(std::string const& fileName, boost::asio::ip::tcp::endpoint const& endpoint, boost::asio::yield_context yield_)
        {
            m_socket.async_connect(endpoint, yield_);
            boost::asio::async_write(m_socket, boost::asio::buffer(fileName), yield_);
            char data[32768] = {0};
            Reporter reporter(m_logger);
            try
            {
                for(; !m_shutdown; )
                {
                    std::size_t count = m_socket.async_read_some(boost::asio::buffer(data), yield_);
                    reporter.OnReceived(count);
                    m_destination.write(data, count);
                }
            }
            catch(boost::system::system_error e)
            {
                if(e.code().value() != 2)
                {
                    throw;
                }
            }
        }

    private:
        log4cpp::Category& m_logger;
        std::ofstream& m_destination;

        boost::asio::io_service m_service;
        boost::asio::ip::tcp::socket m_socket;
        bool m_shutdown;
        std::future<void> m_serviceThread;
    };

}

namespace tsClient
{

    TS_CLIENT_API Client* TsCreate(std::string const& address, short port, std::string const& fileName, std::ofstream& destination, log4cpp::Category& logger)
    {
        return new ClientImp(address, port, fileName, destination, logger);
    }

    TS_CLIENT_API void TsDestroy(Client* client)
    {
        delete client;
    }

}
