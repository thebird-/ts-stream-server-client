#define TS_SERVER_ENGINE_IMPLEMENTATION
#include "TsServerEngine/TsServerEngine.h"

#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <algorithm>
#include <future>
#pragma warning(push)
#pragma warning(disable: 4706)
#include "log4cpp/Category.hh"
#pragma warning(pop)
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"

#include "boost/asio.hpp"
#include "boost/make_unique.hpp"
#include "boost/asio/spawn.hpp"
#include "boost/asio/system_timer.hpp"
#include "boost/format.hpp"

#include "Report.h"
#include "SourceFile.h"
#include "SessionManagerCallback.h"
#include "Session.h"
#include "Misc.h"

#ifdef _DEBUG
#define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_CLIENTBLOCK
#endif // _DEBUG

#ifdef _DEBUG
#define new DEBUG_CLIENTBLOCK
#endif

namespace
{
    class SessionManager
        : public tsServer::Server
        , public SessionManagerCallback
    {
    public:
        SessionManager(short port, std::string const& sourceFolder, log4cpp::Category& logger)
            : m_sourceFolder(sourceFolder)
            , m_logger(logger)
            , m_clientCounter(0)
            , m_reporter(std::chrono::seconds(3), m_clientCounter)
            , m_acceptor(m_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
            , m_timerReport(m_service)
            , m_shutdown(false)
        {
            spawn(m_service, [this](boost::asio::yield_context yield_) { acceptor(yield_); }, m_logger, "Session Manager Acceptor");
            spawn(m_service, [this](boost::asio::yield_context yield_) { reporter(yield_); }, m_logger, "Session Manager Reporter");
            m_serviceThread = startIoService(m_service, m_logger, "Session Manager");
        }

        virtual ~SessionManager()
        {
            m_service.post([this]()
            {
                cleanup();
            });
            m_serviceThread.wait();
        }

    private:
        void acceptor(boost::asio::yield_context yield_)
        {
            intptr_t token = 0;
            for(;;)
            {
                std::unique_ptr<boost::asio::io_service> service(new boost::asio::io_service);
                boost::asio::ip::tcp::socket socket(*service);
                m_acceptor.async_accept(socket, yield_);
                if(m_shutdown)
                {
                    throw boost::system::system_error(boost::asio::error::operation_aborted);
                }
                socket.set_option(boost::asio::ip::tcp::no_delay(true));
                m_sessions[token] = std::make_shared<Session>(*this, ++token, std::move(service), std::move(socket));
                m_clientCounter++;
            }
        }

        void reporter(boost::asio::yield_context yield_)
        {
            for(;;)
            {
                m_timerReport.expires_from_now(std::chrono::seconds(3));
                m_timerReport.async_wait(yield_);
                if(m_shutdown)
                {
                    throw boost::system::system_error(boost::asio::error::operation_aborted);
                }
                m_reporter.DoReport(m_logger.infoStream(), false);
            }
        }

        void cleanup()
        {
            m_shutdown = true;
            m_acceptor.cancel();
            std::for_each(m_sessions.begin(), m_sessions.end(), [](std::map<intptr_t, std::shared_ptr<Session>>::value_type& session)
            {
                session.second->Cancel();
            });
            m_sessions.clear();
        }

        void removeSession(intptr_t token)
        {
            m_sessions.erase(token);
            m_clientCounter--;
        }

    private:
        virtual log4cpp::Category& GetLogger() override
        {
            return m_logger;
        }

        virtual std::string const& GetSourceFolder() override
        {
            return m_sourceFolder;
        }

        virtual Report& GetReporter() override
        {
            return m_reporter;
        }

        virtual void OnSessionFinished(intptr_t token) override
        {
            m_acceptor.get_io_service().post([this, token]()
            {
                removeSession(token);
            });
        }

    private:
        std::string const& m_sourceFolder;
        log4cpp::Category& m_logger;
        std::atomic_uint m_clientCounter;
        ReportTotal m_reporter;
        boost::asio::io_service m_service;
        std::future<void> m_serviceThread;
        boost::asio::ip::tcp::acceptor m_acceptor;
        boost::asio::system_timer m_timerReport;
        std::map<intptr_t, std::shared_ptr<Session>> m_sessions;
        std::atomic_bool m_shutdown;
    };

}

namespace tsServer
{

    Server* TsCreate(short port, std::string const& source, log4cpp::Category& logger)
    {
        return new SessionManager(port, source, logger);
    }

    void TS_SERVER_API TsDestroy(Server* server)
    {
        delete server;
    }
}
