# include "Commands.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"
# include "InputParser.hpp"

Command::Command(CommandType type, Server& server) : _type(type), _server(server) {
	commands[CAP] = &Command::handleCap;
	commands[PASS] = &Command::handlePass;
	commands[NICK] = &Command::handleNick;
	commands[USER] = &Command::handleUser;
	commands[QUIT] = &Command::handleQuit;
	commands[PING] = &Command::handlePing;
	commands[PONG] = &Command::handlePong;
	commands[OPER] = &Command::handleOper;
	commands[PRIVMSG] = &Command::handlePrivMsg;
	commands[JOIN] = &Command::handleJoin;
	commands[TOPIC] = &Command::handleTopic;
	commands[PART] = &Command::handlePart;
	commands[KICK] = &Command::handleKick;
	commands[INVITE] = &Command::handleInvite;
	commands[MODE] = &Command::handleMode;
}

Command::~Command() {}

void Command::execute(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	if (commands.find(_type) != commands.end()) {
		(this->*commands[_type])(client, args, channels);
	}
}

void Command::handleCap(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	(void) client;
	(void) args;
	(void) channels;
	return ;
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
		client.sendMessage("You have been authenticated. Please continue your registration.");
		if (!client.nickname.empty() && !client.username.empty()) {
			response = RPL_WELCOME(client.nickname);
			client.sendMessage(response);
		}
	} else {
		response = ERR_PASSWDMISMATCH;
		client.sendMessage(response);
	}
}

void Command::handleNick(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	std::string newNick = trim(args);
	std::string oldNick = client.nickname;
	bool nickExists = false;

	// Checks if the Client is already authenticated
	if (!client.isAuthenticated()) {
		client.sendMessage(ERR_NOTREGISTERED);
		return;
	}

	//check for a valid newNickname : ERR_ERRONEUSNICKNAME 432
	std::map<int, Client*>& clients = _server.getClients();
	for (ClientsIte it = clients.begin(); it != clients.end(); it++) {
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

		// Inform other clients about the nickname change
		// std::string message = ":" + oldNick + "!@localhost NICK: " + newNick;
		for (ChannelIte itChannel = channels.begin(); itChannel != channels.end(); itChannel++) {
			Channel* channel = itChannel->second;
			if (channel->isMember(&client)) {
				channel->broadcast(response, &client);
			}
		}
	}
	// Check if registration OK
	if (!client.username.empty()) {
		client.setRegistered(true);
	} 
	std::cout << "NICK command received. Client nickname changed from: " << oldNick << " to: " << newNick << std::endl;
}


void Command::handleUser(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	(void)channels;

	// Checks if the Client is already authenticated
	if (!client.isAuthenticated()) {
		client.sendMessage(ERR_NOTREGISTERED);
		return;
	}
	std::vector<std::string> tokens = InputParser::parseInput(args, ' ');

	if (tokens.size() < 4) {
		std::string command = "USER";
		std::string response = ERR_NEEDMOREPARAMS(command);
		client.sendMessage(response);
		client.sendMessage("Usage: Command <username> 0 * <realname>\r\n");
		return ;
	}

	std::string userName = trim(tokens[0]);
	std::string mode = trim(tokens[1]);
	std::string unused = trim(tokens[2]);
	std::string realName = trim(tokens[3]);

	if (!realName.empty() && realName[0] == ':') {
		realName = realName.substr(1);
	}

	if (userName.empty() || userName.length() > USERLEN) {
		std::string command = "USER";
		std::string response = ERR_NEEDMOREPARAMS(command);
		client.sendMessage(response);
		return ;
	}

	std::map<int, Client*>& clients = _server.getClients();
	for (ClientsIte it = clients.begin(); it != clients.end(); it++) {
		if (client.username == userName) {
			std::string response = ERR_ALREADYREGISTERED;
			client.sendMessage(response);
			return ;
		}
	}

	client.username = userName;
	client.realname = realName;

	// Check if registration OK
	if (!client.nickname.empty()) {
		client.setRegistered(true);
	} 
	std::cout << "USER command received. Client username set to: " << userName << ", realname set to: " << realName << std::endl;
}

void Command::handleQuit(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	std::vector<std::string> tokens = InputParser::parseInput(args, ' ');
	std::string reason;
	std::string response;

	if (tokens.empty()) {
		reason = "";
	} else {
		for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
			reason.append(*it);
			if (it + 1 != tokens.end()) {
				reason.append(" ");
			}
		}
	}
	if (reason.empty()) {
		response = "Quit";
	} else {
		response = RPL_QUIT(reason);
	}
	// todo: Check if this is the right approach for deletion
	for (ChannelIte it = channels.begin(); it != channels.end(); it++) {
		it->second->broadcast(client.nickname + " has quit: " + reason, &client);
		it->second->removeMember(&client);
	}
	
	client.sendMessage(response);
	throw std::runtime_error("Client disconnected");
	std::cout << "QUIT command received. Client disconnected with the reason: " << response << std::endl;
}

/* PING command is sent by either clients or servers to check the other
 side of the connection is still connected. */
void Command::handlePing(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	(void)channels;

	std::vector<std::string> tokens = InputParser::parseInput(args, ' ');
	std::string reason = tokens.empty() ? "" : trim(tokens[0]);

	InputParser::printTokens(tokens);

	if (reason.empty()) {
		std::string command = "PING";
		std::string response = ERR_NEEDMOREPARAMS(command);
		client.sendMessage(response);
		return;
	}
	client.sendMessage("PONG " + reason + "\r\n");
}


void Command::handlePong(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	(void)channels;

	std::vector<std::string> tokens = InputParser::parseInput(args, ' ');
	std::string reason = tokens.empty() ? "" : trim(tokens[0]);

	if (reason.empty()) {
		std::string command = "PING";
		std::string response = ERR_NEEDMOREPARAMS(command);
		client.sendMessage(response);
		return;
	}
	std::cout << "PONG command received with the token: " << reason  << std::endl;
}

void Command::handleOper(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	(void)channels;

	// Checks if the Client is already authenticated
	if (!client.isAuthenticated() || client.nickname.empty() || client.username.empty()) {
		client.sendMessage(ERR_NOTREGISTERED);
		return;
	}

	std::string command = "OPER";
	std::string response;
	std::vector<std::string> tokens = InputParser::parseInput(args, ' ');

	if (tokens.size() < 2) {
		client.sendMessage(ERR_NEEDMOREPARAMS(command));
		client.sendMessage("Usage: Command <name> <password>\r\n");
		return ;
	}

	std::string name = trim(tokens[0]);
	std::string password = trim(tokens[1]);

	if (name.empty() || password.empty()) {
		response = ERR_NEEDMOREPARAMS(command);
		client.sendMessage(response);
		return ;
	}
	if (name != _server.getOperName()) {
		response = ERR_NOOPERHOST;
		client.sendMessage(response);
		return ;
	}
	if (password != _server.getOperPassword()) {
		response = ERR_PASSWDMISMATCH;
		client.sendMessage(response);
		return ;
	}

	client.setServerOperator(true);
	response = RPL_YOUREOPER;
	client.sendMessage(response);
	std::cout << "Client " << client.nickname << " is now a server operator." << std::endl;
}

void Command::handlePrivMsg(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	std::string command = "PRIVMSG";

	// Checks if the Client is already authenticated
	// todo: receive messages until registration is complete
	if (!client.isAuthenticated() || client.nickname.empty() || client.username.empty()) {
		client.sendMessage(ERR_NOTREGISTERED);
		return;
	}

	// Find the position of the colon that tells me the start of the message
	size_t colonPos = args.find(" :");
	if (colonPos == std::string::npos) {
		client.sendMessage(ERR_NEEDMOREPARAMS(command));
		client.sendMessage("Usage: Command <target>{,<target>} :<text to be sent>\r\n");
		return;
	}

	// Extract the targets and the message
	std::string targetsStr = args.substr(0, colonPos);
	std::string message = args.substr(colonPos + 2); // Skipping the " :"

	// Split the target by comma
	std::vector<std::string> tokens = InputParser::parseInput(targetsStr, ',');

	// Debug printing
	InputParser::printTokens(tokens);
	std::cout << "Message: " << message << std::endl;

	for (std::vector<std::string>::iterator itVector = tokens.begin(); itVector != tokens.end(); itVector++) {
		bool found = false;
		std::string target = *itVector;

		// Check if the target is a channel
		if (target[0] == '#') {
			Channel* channel = channels[target];
			if (channel) {
				channel->broadcast(client.nickname + ": " + message, &client);
				found = true;
			} else {
				client.sendMessage(ERR_NOSUCHCHANNEL(target));
			}
		} else {
			std::map<int, Client*>& clients = _server.getClients();
			for (ClientsIte itClient = clients.begin(); itClient != clients.end(); itClient++) {
				if (itClient->second->nickname == target) {
					itClient->second->sendMessage(client.nickname + " :" + message);
					found = true;
					break;
				}
			}
			if (!found) {
				client.sendMessage(ERR_NOSUCKNICK(target));
			}
		}
	}
}

