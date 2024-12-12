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
	if (commandName[0] == '#') {
		Client *clientPtr = &client;
		bool isMember = false;
		Channel* targetChannel = NULL;
		for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
			if (it->first == commandName) {
				targetChannel = it->second;
				std::vector<Client*> members = targetChannel->getMembers(); // Get the members vector
				for (std::vector<Client*>::iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt) {
					if (*memberIt == clientPtr) {
						isMember = true;
						break;
					}
				}
				break;
			}
		}
		if (isMember) {
			targetChannel->broadcast(args, clientPtr);
		} else {
			std::string error = "You are not a member of this channel.\n";
			clientPtr->sendMessage(error);
		}
		return;
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
