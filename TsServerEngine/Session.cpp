#include "Session.h"

#include "boost/format.hpp"

#include "SourceFile.h"
#include "Report.h"
#include "Misc.h"

Session::Session(SessionManagerCallback& server, intptr_t token, std::unique_ptr<boost::asio::io_service>&& service, boost::asio::ip::tcp::socket&& socket)
    : m_service(std::move(service))
    , m_server(server)
    , m_token(token)
    , m_socket(std::move(socket))
    , m_timer(*m_service)
    , m_work(*m_service)
{
    spawn(*m_service, [this](boost::asio::yield_context yield_) { session(yield_); }, m_server.GetLogger(), (boost::format("Session %1%") % token).str());
    m_serviceThread = startIoService(*m_service, m_server.GetLogger(), (boost::format("Session %1%") % token).str());
}

void Session::Cancel()
{
    m_service->post([this]()
    {
        m_socket.close();
    });
}

void Session::session(boost::asio::yield_context yield_)
{
    std::shared_ptr<void> finisher(&m_token, [this](intptr_t* token)
    {
        m_server.OnSessionFinished(*token);
    });

    std::shared_ptr<boost::asio::ip::tcp::socket> socketGuard(&m_socket, [this](boost::asio::ip::tcp::socket* socket)
    {
        try
        {
            socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        }
        catch(boost::system::system_error e)
        {
            m_server.GetLogger().errorStream() << boost::format("Session %3%: thread id: %2%, error: %1%") % e.code().message() % std::this_thread::get_id() % m_token;
        }
        catch(std::exception e)
        {
            m_server.GetLogger().errorStream() << boost::format("Session %3%: thread id: %2%, error: %1%") % e.what() % std::this_thread::get_id() % m_token;
            throw;
        }
    });

    std::string const fileName = readPath(yield_);
    SourceFile source(fileName);

    std::vector<char> buffer;
    TimePoint beginSend(Clock::now());
    ReportChannel reporter(std::chrono::seconds(3), fileName);
    for(;;)
    {
        source.Read(buffer);

        sendData(m_socket, buffer, yield_);

        m_server.GetReporter().OnSent(buffer.size());

        reporter.OnSent(buffer.size());
        reporter.DoReport(m_server.GetLogger().infoStream(), source.IsEof());

        if(source.IsEof())
        {
            break;
        }
        waitNextTick(m_timer, beginSend + source.GetCurrentDuration(), yield_);
    }
}

std::string Session::readPath(boost::asio::yield_context yield_)
{
    boost::asio::streambuf buffer;
    boost::system::error_code error;
    boost::asio::async_read_until(m_socket, buffer, '\0', yield_);
    std::ostringstream out;
    out << &buffer;
    std::string path = m_server.GetSourceFolder() + '/' + std::string(out.str().c_str());
#if(defined(_MSC_VER) || defined(__MINGW32__))
    std::replace(path.begin(), path.end(), '/', '\\');
#endif
    return path;
}

void Session::waitNextTick(boost::asio::system_timer& timer, TimePoint const& tick, boost::asio::yield_context yield_)
{
    if(tick > std::chrono::system_clock::now())
    {
        timer.expires_at(tick);
        timer.async_wait(yield_);
    }
}

void Session::sendData(boost::asio::ip::tcp::socket& socket, std::vector<char> const& buffer, boost::asio::yield_context yield_)
{
    boost::asio::async_write(socket, boost::asio::buffer(buffer), yield_);
}
