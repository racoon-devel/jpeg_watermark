
#include "easylogging++.h"
#include "server.hpp"

INITIALIZE_EASYLOGGINGPP

int main()
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();

    defaultConf.setGlobally(
        el::ConfigurationType::Format, "%datetime [%level] %msg");
        el::Loggers::reconfigureLogger("default", defaultConf);

    LOG(INFO) << "Watermerk JPEG server";

    Server server("127.0.0.1", 9001, 8);
    
    return server.Run();
}