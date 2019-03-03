
#include "easylogging++.h"
#include "server.hpp"
#include "cxxopts.hpp"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char **argv)
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();

    defaultConf.setGlobally(
        el::ConfigurationType::Format, "%datetime [%level] %msg");
        el::Loggers::reconfigureLogger("default", defaultConf);

    LOG(INFO) << "Watermark JPEG server";

    // Parse options
    cxxopts::Options options("Watermark JPEG Server", "Server for drawing watermarks");

    options.add_options()
        ("a,addr", "Address", cxxopts::value<std::string>()->default_value("127.0.0.1"), "Server IPv4 address")
        ("p,port", "Port", cxxopts::value<int>()->default_value("9001"), "Server port")
        ("L,limit", "Limit", cxxopts::value<uint>()->default_value("10"), "Maximum jobs limitation")
        ("help", "Print help")
        ;

    std::string addr; int port; uint max_jobs;

    try
    {
        auto opts = options.parse(argc, argv);

        if (opts.count("help"))
        {
            std::cout << options.help() << std::endl;
            return 1;
        }

        addr = opts["addr"].as<std::string>();
        port = opts["port"].as<int>();
        max_jobs = opts["limit"].as<uint>();
        
    }
    catch(const cxxopts::OptionException& e)
    {
        LOG(ERROR) << e.what();
        return 1;
    }

    if (!port || !max_jobs)
    {
        LOG(ERROR) << "Invalid arguments";
        return 1;
    }

    Server server(addr, port, max_jobs);
    return server.Run();
}