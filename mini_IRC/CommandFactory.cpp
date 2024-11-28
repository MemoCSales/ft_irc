# include "CommandFactory.hpp"

typedef std::map<std::string, CommandCreator*>::iterator itBegin;
typedef std::map<std::string, CommandCreator*>::iterator itFind;

CommandFactory::CommandFactory() {
	commands["CAP"] = new CommandCreatorImpl<Command>(CAP);
	commands["PASS"] = new CommandCreatorImpl<Command>(PASS);
	commands["NICK"] = new CommandCreatorImpl<Command>(NICK);
	commands["USER"] = new CommandCreatorImpl<Command>(USER);
	commands["QUIT"] = new CommandCreatorImpl<Command>(QUIT);
}

CommandFactory::~CommandFactory() {
	for (itBegin it = commands.begin(); it != commands.end(); it++)	{
		delete it->second;
	}
	
}

CommandPtr CommandFactory::createCommand(const std::string& commandName) {
	// todo: move commands map here instead
	itFind it = commands.find(commandName);
	if (it != commands.end()) {
		return (*(it->second))();
	}
	return NULL;
}
