# include "CommandFactory.hpp"

typedef std::map<std::string, CommandCreator*>::iterator itBegin;
typedef std::map<std::string, CommandCreator*>::iterator itFind;

CommandFactory::CommandFactory(Server& server) {
	commands["CAP"] = new CommandCreatorImpl<Command>(CAP, server);
	commands["PASS"] = new CommandCreatorImpl<Command>(PASS, server);
	commands["NICK"] = new CommandCreatorImpl<Command>(NICK, server);
	commands["USER"] = new CommandCreatorImpl<Command>(USER, server);
	commands["QUIT"] = new CommandCreatorImpl<Command>(QUIT, server);
	commands["PING"] = new CommandCreatorImpl<Command>(PING, server);
	commands["PONG"] = new CommandCreatorImpl<Command>(PONG, server);
	commands["OPER"] = new CommandCreatorImpl<Command>(OPER, server);
	commands["PRIVMSG"] = new CommandCreatorImpl<Command>(PRIVMSG, server);
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
