#ifndef COMMAND_PARSER_HPP
# define COMMAND_PARSER_HPP

// # include "CommandFactory.hpp"
# include "Client.hpp"

class Client;
class CommandFactory;

class CommandParser {
	private:
		CommandFactory* commandFactory;
	public:
		CommandParser();
		~CommandParser();
		void parseAndExecute(Client& client, const std::string& message);
};

#endif
