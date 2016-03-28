#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include "config.h"

void worker_main(boost::shared_ptr<boost::asio::io_service> io_service) {
	io_service->run();
}

void on_read(const boost::system::error_code &ec, size_t bytes_read,
             boost::shared_ptr<boost::asio::io_service::strand> strand,
             boost::shared_ptr<boost::asio::ip::tcp::socket> socket,
             boost::shared_ptr<boost::asio::streambuf> buffer) {
	if(ec) {
		std::cout << "error " << ec << std::endl;
	} else {
		std::cout << "<- " << buffer;
		std::string delimiter("\r\n");
		boost::asio::async_read_until(*socket, *buffer, delimiter,
		                              strand->wrap(boost::bind(on_read, _1, _2, strand, socket, buffer)));
	}
}

void on_write(const boost::system::error_code &ec, size_t bytes_written) {
	if(ec) {
		std::cout << "error " << ec << std::endl;
	} else {
		std::cout << "->" << bytes_written << std::endl;
	}
}

void on_connect(const boost::system::error_code &ec,
                boost::shared_ptr<boost::asio::io_service::strand> strand,
                boost::shared_ptr<boost::asio::ip::tcp::socket> socket) {
	if(ec) {
		std::cout << "error " << ec << std::endl;
	} else {
		std::cout << "connected" << std::endl;
		try {
			boost::asio::async_write(*socket, boost::asio::buffer(std::string("NICK ibotpp\r\n")), on_write);
			boost::asio::async_write(*socket, boost::asio::buffer(std::string("USER ibotpp 0 * :ibot++\r\n")), on_write);
			auto buffer = boost::make_shared<boost::asio::streambuf>();
			std::string delimiter("\r\n");
			boost::asio::async_read_until(*socket, *buffer, delimiter,
			                              strand->wrap(boost::bind(on_read, _1, _2, strand, socket, buffer)));
		} catch(std::exception &e) {
			std::cout << "exception" << std::endl;
		}
	}
}

int main(int argc, char *argv[]) {
	auto io_service = boost::make_shared<boost::asio::io_service>();
	auto work = boost::make_shared<boost::asio::io_service::work>(*io_service);
	auto strand = boost::make_shared<boost::asio::io_service::strand>(*io_service);
	boost::thread_group workers;
	for(unsigned int w = 0; w < CONFIG_NUM_WORKERS; ++w) {
		workers.create_thread(boost::bind(&worker_main, io_service));
	}

	auto socket = boost::make_shared<boost::asio::ip::tcp::socket>(*io_service);
	boost::asio::ip::tcp::resolver resolver(*io_service);
	boost::asio::ip::tcp::resolver::query query(CONFIG_HOSTNAME, boost::lexical_cast<std::string>(CONFIG_PORT));
	auto it = resolver.resolve(query);
	boost::asio::ip::tcp::endpoint endpoint = *it;
	std::cout << "connecting to " << endpoint << std::endl;
	socket->async_connect(endpoint, strand->wrap(boost::bind(on_connect, _1, strand, socket)));

	boost::system::error_code ec;
	work.reset();
	workers.join_all();
	socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket->close(ec);
	return 0;
}
