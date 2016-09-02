#include "SourceFile.h"

#include "boost/system/system_error.hpp"

namespace
{
    std::ifstream openFile(std::string const& path)
    {
        std::ifstream ifs;
        ifs.open(path.c_str(), std::ios::binary);
        if(ifs.fail())
        {
            throw boost::system::system_error(errno, boost::system::system_category());
        }
        return ifs;
    }

    double calculateBytesPerSecond(std::string const& path)
    {
        std::ifstream ifs;
        ifs.open((path + std::string(".bitrate")).c_str());
        if(ifs.fail())
        {
            throw boost::system::system_error(errno, boost::system::system_category());
        }
        double bitrate;
        ifs >> bitrate;
        return bitrate / 8.0;
    }
}

SourceFile::SourceFile(std::string const& path)
    : m_stream(openFile(path))
    , m_bytesPerSecond(calculateBytesPerSecond(path))
    , m_total(0)
    , m_buffer(65536)
    , m_read(std::async([this]() { doRead(); }))
    , m_eof(false)
{
    wait();
    if(m_buffer.size() < 188)
    {
        throw boost::system::system_error(ENODATA, boost::system::system_category());
    }
    m_buffer.erase(m_buffer.begin(), std::find(m_buffer.begin(), m_buffer.begin() + 188, 0x47));
    if(m_buffer.size() < 188 * 5)
    {
        throw boost::system::system_error(ENODATA, boost::system::system_category());
    }
    for(size_t i = 0; i < 5; ++i)
    {
        if(m_buffer[i * 188] != 0x47)
        {
            throw boost::system::system_error(ENODATA, boost::system::system_category());
        }
    }
}

std::chrono::milliseconds SourceFile::GetCurrentDuration()
{
    return std::chrono::milliseconds(static_cast<int>(std::round(m_total / m_bytesPerSecond * 1000)));
}

void SourceFile::Read(std::vector<char>& destination)
{
    wait();
    m_total += m_buffer.size();
    m_buffer.swap(destination);
    m_eof = m_stream.eof() && m_buffer.size() == 0;
    if(!m_eof)
    {
        m_read = std::async([this]()
        {
            doRead();
        });
    }
}

bool SourceFile::IsEof()
{
    return m_eof;
}

void SourceFile::doRead()
{
    try
    {
        m_buffer.resize(65536);
        m_stream.read(&m_buffer[0], m_buffer.size());
        if(m_stream.fail() && !m_stream.eof())
        {
            throw boost::system::system_error(errno, boost::system::system_category());
        }
        m_buffer.resize(static_cast<size_t>(m_stream.gcount()));
    }
    catch(...)
    {
        m_readException = std::current_exception();
    }
}

void SourceFile::wait()
{
    m_read.wait();
    if(m_readException)
    {
        std::rethrow_exception(m_readException);
    }
}