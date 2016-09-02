#pragma once

#include <string>
#include <vector>
#include <future>
#include <fstream>

class SourceFile
{
public:
    SourceFile(std::string const& path);
    std::chrono::milliseconds GetCurrentDuration();
    void Read(std::vector<char>& destination);
    bool IsEof();

private:
    void doRead();
    void wait();

private:
    std::ifstream m_stream;
    double m_bytesPerSecond;
    int64_t m_total;
    std::exception_ptr m_readException;
    std::vector<char> m_buffer;
    std::future<void> m_read;
    bool m_eof;
};
