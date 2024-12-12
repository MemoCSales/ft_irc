# include "Commands.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"
//#include "Client.hpp"

Command::Command(CommandType type, Server& server) : _type(type), _server(server) {
	commands[PASS] = &Command::handlePass;
	commands[NICK] = &Command::handleNick;
	commands[USER] = &Command::handleUser;
	commands[QUIT] = &Command::handleQuit;
	commands[JOIN] = &Command::handleJoin;
	commands[TOPIC] = &Command::handleTopic;
	commands[PART] = &Command::handlePart;
	commands[KICK] = &Command::handleKick;
	commands[INVITE] = &Command::handleInvite;
	commands[MODE] = &Command::handleMode;


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

void Command::handleNick(Client &client, const std::string &args, std::map<std::string, Channel *> &channels) {
	(void) channels;
	std::string newNick = trim(args);
	std::string oldNick = client.nickname;
	bool nickExists = false;

	//check for a valid newNickname : ERR_ERRONEUSNICKNAME 432
	std::map<int, Client *> &clients = _server.getClients();
	std::map<int, Client *>::iterator it = clients.begin();
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


//----------------------test------------------------
void Command::handleJoin(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	std::string channelName = trim(args);
	if (channelName.empty()) {
		std::string error = "Error: No channel name provided.\n";
		client.sendMessage(error);
		return;
	}
	Channel* targetChannel = NULL;
	bool found = false;
	std::map<std::string, Channel*>::iterator it = channels.begin();
	for (; it != channels.end(); ++it) {
		if (it->first == channelName) {
			found = true;
			targetChannel = it->second;
		}
	}

	if (!found) {
		targetChannel = _server.getOrCreateChannel(channelName);
		targetChannel->addMember(&client);
		targetChannel->addOperator(&client);
		std::cout << channelName << " was created!.\n";
		return;
	} else {
		// Check if the client is in channe;
		for (std::vector<Client*>::const_iterator it = targetChannel->getMembers().begin();
			it != targetChannel->getMembers().end(); ++it) {
			if (*it == &client) {
				std::string error = "Already in the channel.\n";
				client.sendMessage(error);
				return;
			}
		}
		// check if chaennel is full
		if (targetChannel->getMembers().size() >= static_cast<size_t>(targetChannel->getLimit())) {
			std::string error = "Channel is full. Unable to join.\n";
			client.sendMessage(error);
			return;
		}

		// check if invitation required
		if (targetChannel->getInviteStatus()) {
			bool invited = false;
			for (std::vector<Client*>::const_iterator it = targetChannel->getAllowedPeople().begin();
				 it != targetChannel->getAllowedPeople().end(); ++it) {
				if (*it == &client) {
					invited = true;
					break;
				}
			}
			if (!invited) {
				std::string error = "This is a private channel, you need an invitation.\n";
				client.sendMessage(error);
				return;
			}
		}
		if (!targetChannel->getPassword().empty()) {
			std::string response = "Private channel, requires a password.\n";
			client.sendMessage(response);
			// wait for password
			char passBuff[4096];
			memset(passBuff, 0, sizeof(passBuff));
			int passBytesRecv = 0;
			// Loop to wait for password input
			while (true) {
				passBytesRecv = recv(client.getFd(), passBuff, sizeof(passBuff) - 1, 0);
				if (passBytesRecv > 0) {
					std::string pass(passBuff, passBytesRecv);
					pass.erase(pass.find_last_not_of(" \n\r\t") + 1);
					if (targetChannel->getPassword() == pass) {
						targetChannel->addMember(&client);
						std::string message = "Joined channel: " + targetChannel->getName() + ".\n";
						client.sendMessage(message);
						std::cout << client.getNick() << " has joined the channel: " << targetChannel->getName() << std::endl;
						if (!targetChannel->getTopic().empty())
							targetChannel->broadcastTopic(&client);
						return;
					} else {
						std::string error = "Incorrect password.\n";
						client.sendMessage(error);
						return;
					}
				}
			}
		}
	}
	// Join the channel
	targetChannel->addMember(&client);
	std::string message = "Joined channel: " + targetChannel->getName() + ".\n";
	client.sendMessage(message);
	std::cout << client.getNick() << " has joined the channel: " << targetChannel->getName() << std::endl;
	if (!targetChannel->getTopic().empty()) {
		targetChannel->broadcastTopic(&client);
	}
}


void	Command::handleKick(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std::string target;
	stream >> target;

	if (channelName.empty() || channelName[0] != '#') {
		std::string error = "Channel name must start with '#' or channel is empty.\n";
		client.sendMessage(error);
		return;
	}
	if (target.empty()) {
		std::string error = "You need to choose a client to kick.\n";
		client.sendMessage(error);
		return;
	}

	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		std::string error = "Channel does not exist.\n";
		client.sendMessage(error);
		return;
	}
	bool isOperator = false;
	for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
		if (targetChannel->getOperators()[i] == &client) {
				isOperator = true;
				break;
		}
	}
	if (isOperator){
		bool found = false;
		for (unsigned long int i = 0; i < targetChannel->getMembers().size(); i++){
			if (targetChannel->getMembers()[i]->getNick() == target) {
				found = true;
				std::string response = "You got kicked from teh channel\n";
				send(targetChannel->getMembers()[i]->getFd(), response.c_str(), response.size() + 1, 0);
				std::cout << targetChannel->getMembers()[i]->getNick() << " got kicked from: " << targetChannel->getName()<<std::endl;
				targetChannel->removeMember(targetChannel->getMembers()[i]);

				break;
			}
		}
		if (!found){
			std::string error = "User not found in the channel.\n";
			client.sendMessage(error);
		}
	}
	else {
		std::string error = "You 're not an operator.\n";
		client.sendMessage(error);
	}
}

void	Command::handlePart(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	if (channelName.empty() || channelName[0] != '#') {
		std::string error = "Channel name must start with '#' or channel is empty.\n";
		client.sendMessage(error);
		return;
	}
	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		std::string error = "Channel does not exist.\n";
		client.sendMessage(error);
		return;
	}
	bool isOperator = false;
	for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
		if (targetChannel->getOperators()[i] == &client) {
			isOperator = true;
			break;
		}
	}
	bool found = false;
	for (unsigned long int i = 0; i < targetChannel->getMembers().size(); i++){
		if (targetChannel->getMembers()[i] == &client) {
			found = true;
			targetChannel->removeMember(&client);
			if(isOperator)
					targetChannel->removeOperator(&client);
			std::string response = "Exited channel:" +targetChannel->getName() + "\n";
			client.sendMessage(response);
			std::cout << client.getNick() << " exited from: " << targetChannel->getName()<<std::endl;
			break;
		}
	}
	if (!found){
		std::string error = "User are not part of that channell.\n";
		client.sendMessage(error);
	}
	if(targetChannel->getMembers().size() == 0){
		std::cout << targetChannel->getName() << " was removed!\n";
		_server.removeChannel(targetChannel->getName());
	}
}

// need to check if the client is in the channel;
void	Command::handleTopic(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std::string topic;
	stream >> topic;
	if (channelName.empty() || channelName[0] != '#') {
		std::string error = "Channel name must start with '#' or channel is empty.\n";
		client.sendMessage(error);
		return;
	}
	if(topic.empty()){
		std::string error = "topic can t be empty.\n";
		client.sendMessage(error);
		return;
	}
	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		std::string error = "Channel does not exist.\n";
		client.sendMessage(error);
		return;
	}
	bool isMember = false;
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		if (it->first == channelName) {
			targetChannel = it->second;
			std::vector<Client*> members = targetChannel->getMembers(); // Get the members vector
			for (std::vector<Client*>::iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt) {
				if (*memberIt == &client) {
					isMember = true;
					break;
				}
			}
			break;
		}
	}
	if(isMember){
		if(targetChannel->getFlagTopic() == true){
			std::string error = "Topic restriction set to true, You need to be an operator.\n";
			client.sendMessage(error);
			bool isOperator = false;
			for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
				if (targetChannel->getOperators()[i] == &client) {
					isOperator = true;
					break;
				}
			}
			if(isOperator){
				targetChannel->setTopic(topic);
				std::string message = "Channel topic set to: " + targetChannel->getTopic()+'\n';
				client.sendMessage(message);
				std::cout << targetChannel->getName() + " has the topic set to: " + targetChannel->getTopic() <<std::endl;
			}
			else{
					std::string error = "You are not an operator\n";
					client.sendMessage(error);
			}
		}
		else{
			targetChannel->setTopic(topic);
			std::string message = "Channel topic set to: " + targetChannel->getTopic()+'\n';
			client.sendMessage(message);
			std::cout << targetChannel->getName() + " has the topic set to: " + targetChannel->getTopic() <<std::endl;
		}
	}
	else{
		std::string message = "You are not a member of this channel\n";
		client.sendMessage(message);
	}
	
}
		

void	Command::handleInvite(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std::string targetNick;
	stream >> targetNick;

	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		std::string error = "Channel does not exist.\n";
		client.sendMessage(error);
		return;
	}

	bool isOperator = false;
	for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
		if (targetChannel->getOperators()[i] == &client) {
			isOperator = true;
			break;
		}
	}
	bool found = false;
	for (unsigned long int i = 0; i < targetChannel->getMembers().size(); i++){
		if (targetChannel->getMembers()[i] == &client) {
			found = true;
			if(isOperator){
				Client *targetClient = _server.getClientByNick(targetNick);
				if (targetClient) {
					targetChannel->addAllowedPeople(targetClient);
					std::string response = "You have been invited to join channel: " + channelName + "\n";
					targetClient->sendMessage(response);
				} else {
					std::string error = "User with nickname " + targetNick + " not found.\n";
					client.sendMessage(error);
				}
			} else {
				std::string error = "You are not an operator in :"  + targetChannel->getName() + " \n";
				client.sendMessage(error);
			}
			break;
		}
	}
	if (!found){
		std::string error = "You are not part of " + targetChannel->getName() +  " channel.\n";
		client.sendMessage(error);
	}
}

void	Command::handleMode(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
		(void)channels;
		std::istringstream stream(args);
		std::string channelName;
		stream >> channelName;
		std:: string flag;
		stream >> flag;
		if (channelName.empty() || channelName[0] != '#') {
			std::string error = "Channel name must start with '#' or channel is empty.\n";
			client.sendMessage(error);
			return;
		}
		//also a check for flag
		Channel *targetChannel = _server.getChannel(channelName);
		if (!targetChannel) {
			std::string error = "Channel does not exist.\n";
			client.sendMessage(error);
			return;
		}
		if(flag == "k")
		{
			std::cout << "pass statement\n";
			std::string passWord;
			stream >> passWord;
			bool isOperator = false;
			for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
				if (targetChannel->getOperators()[i] == &client) {
					isOperator = true;
					break;
				}
			}
			if(isOperator) {
				Channel* targetChannel = NULL;
				bool found = false;
				std::map<std::string, Channel*>::iterator it = channels.begin();
				for (; it != channels.end(); ++it) {
					if (it->first == channelName) {
						found = true;
						targetChannel = it->second;
					}
				}
				if (found){
					targetChannel->setPassword(passWord);
					std::string error = "Password for the channel: " + channelName +" set to: " +passWord + "\n";
					client.sendMessage(error);
				}
				else {
					std::string error = "No channel named :"+ channelName +"\n";
					client.sendMessage(error);
				}
			}
			else {
				std::string error = "You are not an operator\n";
				client.sendMessage(error);
			}

		}
		else if(flag == "i") // nee to check is st or not set after the flag
		{
			std::string mode;
			stream >> mode;
			bool isOperator = false;
			for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
				if (targetChannel->getOperators()[i] == &client) {
					isOperator = true;
					break;
				}
			}
			if(isOperator){
				if(mode == "set"){
					targetChannel->setInviteStatus(true);
					std::string error = "Channeel set to invitation only.\n";
					client.sendMessage(error);
				}
				else if (mode == "remove")
				{
					targetChannel->setInviteStatus(false);
					std::string error = "Invitation only removed.\n";
					client.sendMessage(error);
				}
					
				else {
					std::string error = "You can only use set or remove.\n";
					client.sendMessage(error);
				}
			}
			else{
				std::string error = "You are not an operator.\n";
				client.sendMessage(error);
			}
		}
		else if(flag == "o"){  //need checks if that guy is in the channel and also add coresponding messages
			std::string name;
			stream>>name;
			bool isOperator = false;
			for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
				if (targetChannel->getOperators()[i] == &client) {
					isOperator = true;
					break;
				}
			}
			if(isOperator){
				Client *client = _server.getClientByNick(name);
				targetChannel->addOperator(client);
				std::cout << "new operator adde in the chanel : " <<  name << std::endl;
			}
			else{
				std::string error = "You are not an operator.\n";
				client.sendMessage(error);
			}
		}
		else if(flag == "l"){
			std::string limit;
			stream >> limit;
			bool isOperator = false;
			for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
				if (targetChannel->getOperators()[i] == &client) {
					isOperator = true;
					break;
				}
			}
			if(isOperator)
			{
				int nb = std::atoi(limit.c_str());
				if(nb >2147483647 )
				{
					std::string error = " you can t insert a number bigger than Max_int\n";
					client.sendMessage(error);
					return ;
				}
				else if(nb < 0)
				{
					std::string error = " you can t insert a negative number\n";
					client.sendMessage(error);
					return ;
				}
				targetChannel->setLimit(nb);
			}
			else{
				std::string error = "You are not an operator.\n";
				client.sendMessage(error);
			}
			
		}
		else if(flag == "t"){
			std::string mode;
			stream >> mode;
			bool isOperator = false;
			for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
				if (targetChannel->getOperators()[i] == &client) {
					isOperator = true;
					break;
				}
			}
			if(isOperator){
				if(mode == "set")
				{
					std::string error = "Topic restriction set.\n";
					client.sendMessage(error);
					targetChannel->setFlagTopic(true);
				}
					
				else if(mode == "remove") {
					std::string error = "You removed topic restriction.\n";
					client.sendMessage(error);
					targetChannel->setFlagTopic(false);
				}
				else {
					std::string error = "You can only use set or remove.\n";
					client.sendMessage(error);
				}
					
			}
			else{
				std::string error = "You are not an operator.\n";
				client.sendMessage(error);
			}

		}
}






// handle password protect
		// if (!targetChannel->getPassword().empty()) {
		// 	std::string response = "Private channel, requires a password.\n";
		// 	client.sendMessage(response);

		// 	// wait for password
		// 	char passBuff[4096];
		// 	memset(passBuff, 0, sizeof(passBuff));
		// 	int passBytesRecv = recv(client.getFd(), passBuff, sizeof(passBuff) - 1, 0);
		// 	if (passBytesRecv > 0) {
		// 		std::string pass(passBuff, passBytesRecv);
		// 		pass.erase(pass.find_last_not_of(" \n\r\t") + 1);
		// 		if (targetChannel->getPassword() == pass) {
		// 			targetChannel->addMember(&client);
		// 			std::cout << client.getNick() << " has joined the channel: " << targetChannel->getName() << std::endl;
		// 			if (!targetChannel->getTopic().empty())
		// 				targetChannel->broadcastTopic(&client);
		// 			return;
		// 		} else {
		// 			std::string error = "Incorrect password.\n";
		// 			client.sendMessage(error);
		// 			return;
		// 		}
		// 	}
		// }