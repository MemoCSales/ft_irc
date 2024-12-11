# include "CommandParser.hpp"
# include "CommandFactory.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"

CommandParser::CommandParser(Server& server) {
	commandFactory = new CommandFactory(server);
}

CommandParser::~CommandParser() {
	delete commandFactory;
}

void CommandParser::parseAndExecute(Client& client, const std::string& message, std::map<std::string, Channel*>& channels) {
	std::istringstream stream(message);
	std::string commandName;
	std::string args;

	stream >> commandName;
	std::getline(stream, args);
	// args = Utils::trim(args);
	
	std::cout << "cmdName: " << commandName << std::endl;
	std::cout << "Args: " << args << std::endl;
	// std::cout << "Args in ascii: ";
	// printAsciiDecimal(args);
	// std::cout << "Client AUTH: " << client.isAuthenticated() << std::endl;
	// std::cout << "Cap Negotiation: " << client.isCapNegotiation() << std::endl;
	if (commandName[0] == '#') {  // need to check if the channel exists
	Client *clientPtr = &client;
		for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
			if (it->first == commandName) {
				it->second->broadcast(args,clientPtr);
				return ;
			}
		}
		std::string error = "Error: No channel with that name.\n";
		client.sendMessage(error);
	}
	else {
		CommandPtr command = commandFactory->createCommand(commandName);
		if (command) {
			command->execute(client, args, channels);
			delete command;
		} else {
			std::string response = ERR_UNKNOWNCOMMAND(commandName);
			client.sendMessage(response);
		}
	}
}
