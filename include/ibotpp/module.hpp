#pragma once
#ifndef IBOTPP_MODULE_HPP
#define IBOTPP_MODULE_HPP

#include <string>

namespace ibotpp {
	class module {
	public:
		const std::string name;
		module(const std::string name) : name(name) {}
	};
}

#define MODULE_DECLARATION extern "C" ibotpp::module * module(void)
#define MODULE(...) MODULE_DECLARATION { return new ibotpp::module(__VA_ARGS__); }

MODULE_DECLARATION;

#endif
