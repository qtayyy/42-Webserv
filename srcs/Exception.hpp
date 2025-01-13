#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

#include "../includes/Webserv.hpp"

class Exception : public std::exception
{
	public:
		explicit Exception(const std::string& message) : _message(message) {}
		virtual const char *what() const throw() { return (_message.c_str()); }
		virtual ~Exception(void) throw() {}

	private:
		std::string _message;
};

#endif