#pragma once

#include <string>

#if defined(_MSC_VER) || defined(__MINGW32__)
#if defined(TS_SERVER_ENGINE_IMPLEMENTATION)
#define TS_SERVER_API __declspec(dllexport)
#else
#define TS_SERVER_API __declspec(dllimport)
#endif
#else
#define TS_SERVER_API
#endif

namespace boost
{
    namespace asio
    {
        class io_service;
    }
}

namespace log4cpp
{
    class Category;
}

namespace tsServer
{
    class TS_SERVER_API Server
    {
        friend TS_SERVER_API void TsDestroy(Server* server);

    protected:
        virtual ~Server() = 0
        {
        };
    };

    TS_SERVER_API Server* TsCreate(short port, std::string const& source, log4cpp::Category& logger);
    TS_SERVER_API void TsDestroy(Server* server);

}