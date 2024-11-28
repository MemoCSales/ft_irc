#ifndef COMMAND_HPP
# define COMMAND_HPP

#include <string>
#include <map>
#include "Client.hpp"
#include "Channel.hpp"
# include <Utils.hpp>

# ifndef DEBUG
#  define DEBUG 0
# endif

class Command
{
	public:
		static void handleCommand(const std::string &command, Client *client, std::map<std::string, Channel*> &channels);
};


// std::ostream& operator << (std::ostream& os, Command& rhs);

#endif // COMMAND_HPP