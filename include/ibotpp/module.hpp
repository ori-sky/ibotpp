#pragma once
#ifndef IBOTPP_MODULE_HPP
#define IBOTPP_MODULE_HPP

#include <functional>
#include <string>
#include <unordered_map>

namespace ibotpp {
	class module {
	public:
		typedef std::function<void()> handler_type;
		typedef std::unordered_map<std::string, handler_type> handlers_type;

		const std::string name;
		handlers_type handlers;

		module(const std::string name, handlers_type handlers)
				: name(name), handlers(handlers) {}

		module(const std::string name,
		       std::initializer_list<handlers_type::value_type> handlers)
				: name(name), handlers(handlers) {}
	};
}

#define MODULE_DECLARATION extern "C" ibotpp::module * module(void)
#define MODULE(...) MODULE_DECLARATION { return new ibotpp::module(__VA_ARGS__); }

MODULE_DECLARATION;

#endif
