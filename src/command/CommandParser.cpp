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
	
	std::cout << "cmdName: " << commandName << std::endl;
	std::cout << "Args: " << args << std::endl;

	CommandPtr command = commandFactory->createCommand(commandName);
	if (command) {
		command->execute(client, args, channels);
		delete command;

		// Check if client has completed the registration
		client.checkAndSendWelcomeMessage();
	} else {
		std::string response = ERR_UNKNOWNCOMMAND(commandName);
		client.sendMessage(response);
	}
}
