# include "Commands.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"

Command::Command(CommandType type, Server& server) : _type(type), _server(server) {
	commands[PASS] = &Command::handlePass;
	commands[NICK] = &Command::handleNick;
	commands[USER] = &Command::handleUser;
	commands[QUIT] = &Command::handleQuit;
}

void Command::execute(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	if (commands.find(_type) != commands.end()) {
		(this->*commands[_type])(client, args, channels);
	}
}

void Command::handlePass(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	(void)channels;
	std::string password = trim(args);
	std::string response;
	if (client.isAuthenticated()) {
		response = ERR_ALREADYREGISTERED;
		client.sendMessage(response);
		return;
	}
	if (password == _server.getPassword()) {
		client.setAuthenticated(true);
		std::cout << "Client authenticated -> fd: " << client.getFd() << std::endl;
	} else {
		response = ERR_PASSWDMISMATCH;
		client.sendMessage(response);
	}
	if (!client.nickname.empty() && !client.username.empty()) {
		std::string response = RPL_WELCOME(client.nickname);
		client.sendMessage(response);
	}
}

void Command::handleNick(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	(void)channels;
	std::string newNick = trim(args);
	std::string oldNick = client.nickname;
	bool nickExists = false;

	//check for a valid newNickname : ERR_ERRONEUSNICKNAME 432
	std::map<int, Client*>& clients = _server.getClients();
	std::map<int, Client*>::iterator it = clients.begin();
	for (; it != clients.end(); it++) {
		if (it->second->nickname == newNick) {
			nickExists = true;
			break;
		}
	}
	if (nickExists) {
		std::string response = ERR_NICKNAMEINUSE(newNick);
		client.sendMessage(response);
	} else {
		client.nickname = newNick;
		std::string user = client.username;
		std::string host = "localhost";

		// Acknowledge the NICK command was successfull and print the new nick
		std::string response = RPL_NICKCHANGE(oldNick, user, host, newNick);
		client.sendMessage(response);

		// todo: Inform other clients about the nickname change

		// Check before sending the welcome message
		if (!client.username.empty() && client.isAuthenticated()) {
			std::string response = RPL_WELCOME(client.nickname);
			client.sendMessage(response);
		}
	}
	std::cout << "NICK command received. Client nickname changed from: " << oldNick << " to: " << newNick << std::endl;
}


void Command::handleUser(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	(void)channels;
	std::istringstream stream(args);
	std::string userName, mode, unused, realName;

	stream >> userName >> mode >> unused;
	std::getline(stream, realName);

	userName = trim(userName);
	mode = trim(mode);
	unused = trim(unused);
	realName = trim(realName);

	if (!realName.empty() && realName[0] == ':') {
		realName = realName.substr(1);
	}

	if (userName.empty() || userName.length() > USERLEN) {
		std::string response = ERR_NEEDMOREPARAMS;
		client.sendMessage(response);
		return ;
	}

	std::map<int, Client*>& clients = _server.getClients();
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); it++) {
		if (client.username == userName) {
			std::string response = ERR_ALREADYREGISTERED;
			client.sendMessage(response);
			return ;
		}
	}

	client.username = userName;
	client.realname = realName;

	// Check before sending the message
	if (!client.nickname.empty() && client.isAuthenticated()) {
		std::string response = RPL_WELCOME(client.nickname);
		client.sendMessage(response);
	}

	std::cout << "USER command received. Client username set to: " << userName << ", realname set to: " << realName << std::endl;
}

void Command::handleQuit(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	std::istringstream stream(args);
	std::string reason, response;

	getline(stream, reason);
	if (reason.empty()) {
		response = "Quit";
		std::cout << response << std::endl;
	} else {
		response = RPL_QUIT(reason);
	}
	// todo: Check if this is the right approach for deletion
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); it++) {
		it->second->broadcast(client.nickname + " has quit: " + reason, &client);
		it->second->removeMember(&client);
	}
	
	client.sendMessage(ERROR(response));
	throw std::runtime_error("Client disconnected");
	std::cout << "QUIT command received. Client disconnected with the reason: " << response << std::endl;
}
