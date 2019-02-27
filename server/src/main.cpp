
#include "easylogging++.h"
#include "server.hpp"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char **argv)
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();

    defaultConf.setGlobally(
        el::ConfigurationType::Format, "%datetime [%level] %msg");
        el::Loggers::reconfigureLogger("default", defaultConf);

    LOG(INFO) << "Watermerk JPEG server";

    if (argc != 4)
    {
        LOG(ERROR) << "Usage: <addr> <port> <max_jobs>";
        return 1;
    }

    std::string addr(argv[1]);
    int port = std::atoi(argv[2]);
    uint max_jobs = std::atoi(argv[3]);

    if (!port || !max_jobs)
    {
        LOG(ERROR) << "Invalid arguments";
        return 1;
    }

    Server server(addr, port, max_jobs);
    
    return server.Run();
}