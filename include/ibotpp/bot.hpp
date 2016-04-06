#pragma once
#ifndef IBOTPP_BOT_HPP
#define IBOTPP_BOT_HPP

#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
//#include <boost/algorithm/string/split.hpp>

namespace ibotpp {
	class bot {
	public:
		typedef boost::asio::ip::tcp::socket socket_type;
	private:
		const boost::shared_ptr<boost::asio::io_service> io_service;
		const boost::shared_ptr<boost::asio::io_service::strand> strand;
		const std::string host;
		const uint16_t port;
		boost::shared_ptr<socket_type> socket;

		void connect(void) {
			socket = boost::make_shared<socket_type>(*io_service);
			boost::asio::ip::tcp::resolver resolver(*io_service);
			boost::asio::ip::tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
			auto it = resolver.resolve(query);
			boost::asio::ip::tcp::endpoint endpoint = *it;
			std::cout << "connecting to " << endpoint << std::endl;
			auto f = boost::bind(&bot::on_connect, this, _1);
			socket->async_connect(endpoint, strand->wrap(f));
		}

		void read_loop(void) {
			auto buffer = boost::make_shared<boost::asio::streambuf>();
			auto f = boost::bind(&bot::on_read, this, _1, _2, buffer);
			boost::asio::async_read_until(*socket, *buffer, "\r\n", strand->wrap(f));
		}

		void write(const std::string line) const {
			std::cout << "<- " << line << std::endl;
			auto f = boost::bind(&bot::on_write, this, _1, _2, line);
			boost::asio::async_write(*socket, boost::asio::buffer(line + "\r\n"), strand->wrap(f));
		}

		void on_connect(const boost::system::error_code &ec) {
			if(ec) {
				std::cout << "connect error " << ec << std::endl;
				connect();
			} else {
				std::cout << "connected" << std::endl;
				write(std::string("NICK ibotpp"));
				write(std::string("USER ibotpp 0 * :ibot++"));
				read_loop();
			}
		}

		void on_write(const boost::system::error_code &ec, size_t bytes_written, const std::string line) const {
			if(ec) { std::cout << "write error " << ec << std::endl; }
		}

		void on_read(const boost::system::error_code &ec, size_t bytes_read,
					 boost::shared_ptr<boost::asio::streambuf> buffer) {
			if(ec) {
				std::cout << "read error" << ec << std::endl;
				connect();
			} else {
				std::cout << "-> " << buffer << std::endl;
				read_loop();
			}
		}
	public:
		bot(decltype(io_service) io_service, decltype(strand) strand,
		    decltype(host) host, decltype(port) port)
				: io_service(io_service), strand(strand), host(host), port(port) {
			connect();
		}
	};
}

#endif
