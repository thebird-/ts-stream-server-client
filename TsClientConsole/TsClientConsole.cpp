#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#else
static_assert(false, "Unsupported OS");
#endif

#include "boost/program_options.hpp"
#include "boost/asio.hpp"

#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"

#include <fstream>

#include "TsClientEngine/TsClientEngine.h"

int main(int argc, char** argv)
{
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "this message")
        ("server", boost::program_options::value<std::string>(), "server address")
        ("port", boost::program_options::value<short>(), "port")
        ("file", boost::program_options::value<std::string>(), "filename on server")
        ("destination", boost::program_options::value<std::string>(), "destination path");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if(vm.count("help") != 0 || vm.count("server") != 1 || vm.count("port") != 1 || vm.count("file") != 1 || vm.count("destination") != 1)
    {
        std::cout << desc << "\n";
        system("pause");
        return 1;
    }

    log4cpp::OstreamAppender appender("console", &std::cout);
    log4cpp::Category& root = log4cpp::Category::getRoot();
#if defined(_DEBUG)
    log4cpp::Priority::PriorityLevel const level = log4cpp::Priority::DEBUG;
#else
    log4cpp::Priority::PriorityLevel const level = log4cpp::Priority::INFO;
#endif
    root.setPriority(level);
    root.addAppender(appender);
    root.debug("Program started");
    root.debug("Server: \"%s\"", vm["server"].as<std::string>().c_str());
    root.debug("Port: \"%d\"", vm["port"].as<short>());
    root.debug("File: \"%s\"", vm["file"].as<std::string>().c_str());
    root.debug("Destination: \"%s\"", vm["destination"].as<std::string>().c_str());
    std::ofstream destination(vm["destination"].as<std::string>(), std::ios::binary);
    boost::shared_ptr<tsClient::Client> client(tsClient::TsCreate(vm["server"].as<std::string>(), vm["port"].as<short>(), vm["file"].as<std::string>(), destination, root), &tsClient::TsDestroy);
    root.debug("Client created");
    system("pause");
    return 0;
}