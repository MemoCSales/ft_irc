#ifndef ICOMMAND_HPP
# define ICOMMAND_HPP
# include "Client.hpp"

class Client;
class ICommand {
	public:
		virtual ~ICommand() {};
		virtual void execute(Client& client, const std::string& args) = 0;
};


#endif
