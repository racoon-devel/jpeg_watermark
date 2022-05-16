
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <io_service.hpp>
using namespace std;

#include "asio.hpp"
#include "easylogging++.h"
#include "client.hpp"
#include "request.hpp"
#include "cxxopts.hpp"

INITIALIZE_EASYLOGGINGPP

bool load_image(const string& filepath, Image& image)
{
	ifstream is(filepath, ios::binary);

	if (is.fail())
		return false;

	streampos fsize = is.tellg();
	is.seekg(0, is.end);
	fsize = is.tellg() - fsize;
	is.seekg(0, is.beg);

	image.resize(fsize);

	is.read(reinterpret_cast< char* >(&image[0]), fsize);

	is.close();
	return true;
}

void save_image(const string& filepath, const Image& image)
{
	ofstream os(filepath, ios::binary);

	if (os.fail())
	{
		cerr << "Open file for write failed" << endl;
		return;
	}

	os.write(reinterpret_cast< const char* >(&image[0]), image.size());

	LOG(INFO) << "Image saved to " << filepath;

	os.close();
}

int main(int argc, char** argv)
{
	// Setup logging
	el::Configurations defaultConf;
	defaultConf.setToDefault();

	defaultConf.setGlobally(el::ConfigurationType::Format,
							"%datetime [%level] %msg");
	el::Loggers::reconfigureLogger("default", defaultConf);

	// Parse options
	cxxopts::Options options("Watermark Client",
							 "Client for the demo JPEG Watermark server");

	Client::Settings settings;
	PrintTextRequest request;

	options.add_options()(
		"a,addr", "Address",
		cxxopts::value< string >()->default_value(settings.address),
		"Server IPv4 address")(
		"p,port", "Port",
		cxxopts::value< int >()->default_value(std::to_string(settings.port)),
		"Server port")("i,image", "Image", cxxopts::value< string >(),
					   "Source image path")(
		"T,text", "Text", cxxopts::value< string >(),
		"Text for drawing")("t,timeout", "Timeout",
							cxxopts::value< uint >()->default_value(
								std::to_string(settings.timeout_sec)),
							"Server reconnect interval")(
		"c,clients", "Clients", cxxopts::value< uint >()->default_value("1"),
		"Clients count")(
		"o,output", "Output",
		cxxopts::value< string >()->default_value("output.jpg"),
		"Image path for result")("help", "Print help");

	uint   clients_count;
	string image_path, output_path;

	try
	{
		auto opts = options.parse(argc, argv);

		if (opts.count("help") || !opts.count("image") || !opts.count("text"))
		{
			cout << options.help() << endl;
			return 1;
		}

		settings.address     = opts["addr"].as< string >();
		settings.port        = opts["port"].as< int >();
		image_path           = opts["image"].as< string >();
		request.text         = opts["text"].as< string >();
		settings.timeout_sec = opts["timeout"].as< uint >();
		output_path          = opts["output"].as< string >();
		clients_count        = opts["clients"].as< uint >();
	}
	catch (const cxxopts::OptionException& e)
	{
		LOG(ERROR) << e.what();
		return 1;
	}

	if (!load_image(image_path, request.image))
	{
		LOG(ERROR) << "Load " << image_path << " failed";
		return 1;
	}

	list< Client > clients;

	for (uint i = 0; i < clients_count; i++)
	{
		clients.emplace_back(settings);
		clients.back().execute(request);
	}

	IoService::get().run();

	uint idx = 0;
	int  ret = 0;

	for (const auto& client : clients)
	{
		if (client.is_success())
		{
			auto file_path = clients_count != 1
				? to_string(idx + 1) + "_" + output_path
				: output_path;

			save_image(file_path, client.image());

			LOG(INFO) << "Image " << file_path << " saved";
		}
		else
		{
			ret = 1;
		}

		idx++;
	}

	return ret;
}