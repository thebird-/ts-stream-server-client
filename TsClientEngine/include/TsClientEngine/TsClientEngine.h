#pragma once

#include <string>

#if defined(_MSC_VER) || defined(__MINGW32__)
#if defined(TS_CLIENT_ENGINE_IMPLEMENTATION)
#define TS_CLIENT_API __declspec(dllexport)
#else
#define TS_CLIENT_API __declspec(dllimport)
#endif
#else
#define TS_CLIENT_API
#endif

namespace log4cpp
{
    class Category;
}

namespace tsClient
{
    class TS_CLIENT_API Client
    {
        friend TS_CLIENT_API void TsDestroy(Client* client);

    protected:
        virtual ~Client() = 0
        {
        };
    };

    TS_CLIENT_API Client* TsCreate(std::string const& address, short port, std::string const& fileName, std::ofstream& destination, log4cpp::Category& logger);
    TS_CLIENT_API void TsDestroy(Client* client);

}