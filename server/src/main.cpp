
#include "easylogging++.h"
#include "cxxopts.hpp"
#include "service.hpp"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char** argv)
{
	el::Configurations defaultConf;
	defaultConf.setToDefault();

	defaultConf.setGlobally(el::ConfigurationType::Format,
							"%datetime [%level] %msg");
	el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
	el::Loggers::reconfigureLogger("default", defaultConf);

	LOG(INFO) << "Watermark JPEG server";

	// Дефолтовые настройки сервиса по умолчанию
	Service::Settings settings;

	using namespace cxxopts;

	// Parse options
	Options options("Watermark JPEG Server",
							 "Server for drawing watermarks");

	options.add_options()(
		"a,addr", "Server IPv4 address", value< std::string >()->default_value(settings.address))
		("p,port", "Port", value< uint >()->default_value(std::to_string(settings.port)))
		("L,limit", "Maximum job limit", value< uint >()->default_value(std::to_string(settings.max_jobs)))
		("help", "Print help");

	try
	{
		auto opts = options.parse(argc, argv);

		if (opts.count("help"))
		{
			std::cout << options.help() << std::endl;
			return 1;
		}

		settings.address  = opts["addr"].as< std::string >();
		settings.port     = opts["port"].as< uint >();
		settings.max_jobs = opts["limit"].as< uint >();
	}
	catch (const OptionException& e)
	{
		LOG(ERROR) << e.what();
		return 1;
	}

	try
	{
		Service service(settings);
		service.run();
		return 0;
	}
	catch (const std::exception& e)
	{
		LOG(FATAL) << e.what();
	}

	return 1;
}