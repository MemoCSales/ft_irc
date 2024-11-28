#ifndef COMMANDS_HPP
# define COMMANDS_HPP
# include "ICommand.hpp"

//todo: make less classes
enum CommandType {
	CAP,
	PASS,
	NICK,
	USER,
	QUIT
};

class Command : public ICommand {
	private:
		CommandType _type;
	public:
		Command(CommandType type) : _type(type) {}
		void execute(Client& client, const std::string& args);
};

#endif
