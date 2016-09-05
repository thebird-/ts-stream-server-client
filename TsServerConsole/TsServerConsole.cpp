#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#else
static_assert(false, "Unsupported OS");
#endif

#include "boost/shared_ptr.hpp"
#include "boost/make_shared.hpp"
#include "boost/program_options.hpp"
#include "boost/asio.hpp"
#include "boost/format.hpp"

#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "TsServerEngine/TsServerEngine.h"

#include <thread>

int main(int argc, char** argv)
{
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "this message")
        ("port", boost::program_options::value<short>(), "port")
        ("source", boost::program_options::value<std::string>(), "source folder");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if(vm.count("help") != 0 || vm.count("port") != 1 || vm.count("source") != 1)
    {
        std::cout << desc << "\n";
        system("pause");
        return 1;
    }

    log4cpp::Category& root = log4cpp::Category::getRoot();
#if defined(_DEBUG)
    log4cpp::Priority::PriorityLevel const level = log4cpp::Priority::DEBUG;
#else
    log4cpp::Priority::PriorityLevel const level = log4cpp::Priority::INFO;
#endif
    root.setPriority(level);
    log4cpp::OstreamAppender consoleAppender("console", &std::cout);
    root.addAppender(consoleAppender);
    log4cpp::FileAppender fileAppender("fa", "fa.log");
    root.addAppender(fileAppender);
    root.debug("Program started");
    root.debug("Source folder: \"%s\"", vm["source"].as<std::string>().c_str());
    root.debug("Port: \"%d\"", vm["port"].as<short>());
    std::unique_ptr<tsServer::Server, void(*)(tsServer::Server*)> server(tsServer::TsCreate(vm["port"].as<short>(), vm["source"].as<std::string>(), root), &tsServer::TsDestroy);
    root.debug("Server created");
    system("pause");
    return 0;
}

