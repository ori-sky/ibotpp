#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <ibotpp/loader.hpp>
#include <ibotpp/bot.hpp>
#include "config.h"

void worker_main(boost::shared_ptr<boost::asio::io_service> io_service) {
	io_service->run();
}

int main(int argc, char *argv[]) {
	ibotpp::loader loader;
	auto module = loader.load("test");
	std::cout << "name = " << module->name << std::endl;
	module->handlers["test"]();

	auto io_service = boost::make_shared<boost::asio::io_service>();
	auto work = boost::make_shared<boost::asio::io_service::work>(*io_service);
	auto strand = boost::make_shared<boost::asio::io_service::strand>(*io_service);
	boost::thread_group workers;
	for(unsigned int w = 0; w < CONFIG_NUM_WORKERS; ++w) {
		workers.create_thread(boost::bind(&worker_main, io_service));
	}
	ibotpp::bot bot(io_service, strand, CONFIG_HOST, CONFIG_PORT);

	work.reset();
	workers.join_all();
	return 0;
}
