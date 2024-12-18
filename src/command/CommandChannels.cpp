# include "Commands.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"
# include "InputParser.hpp"

void Command::handleJoin(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	std::string channelName = trim(args);
	// Check if client registration is complete before joining a channel
	if (!client.isRegistered()) {
		client.sendMessage("Complete your registration before joining a channel");
		return;
	}
	if (channelName.empty()) {
		std::string error = "Error: No channel name provided.\n";
		client.sendMessage(error);
		return;
	}

	// Check for long string. If found truncated
	channelName = Utils::truncateString(channelName);
	
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
		Utils::safePrint(toStr(channelName) + " was created!");
		return;
	} else {
		// Check if the client is in channel;
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
						Utils::safePrint(client.getNick() + " has joined the channel: " + toStr(targetChannel->getName()));
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
	Utils::safePrint(client.getNick() + " has joined the channel: " + toStr(targetChannel->getName()));
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
				std::string response = "You got kicked from the channel:" + channelName + "\n";
				send(targetChannel->getMembers()[i]->getFd(), response.c_str(), response.size() + 1, 0);
				Utils::safePrint(toStr(targetChannel->getMembers()[i]->getNick()) + " got kicked from: " + toStr(targetChannel->getName()));
				targetChannel->removeMember(targetChannel->getMembers()[i]);
				break;
			}
		}
		if (!found){
			std::string error = "User not found in the channel " + channelName + ".\n";
			client.sendMessage(error);
		}
	}
	else {
		std::string error = "You 're not an operator in " + channelName +  "\n";
		client.sendMessage(error);
	}
}

void	Command::handlePart(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	// check if other operators in channel , else give operator priviliges to oldest in channel
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
			{
				targetChannel->removeOperator(&client);

			}
			std::string response = "Exited channel:" +targetChannel->getName() + "\n";
			client.sendMessage(response);
			Utils::safePrint(toStr(client.getNick()) + " exited from: " + targetChannel->getName());
			break;
		}
	}
	if (!found){
		std::string error = "User are not part of that channell.\n";
		client.sendMessage(error);
		return ;
	}
	if(targetChannel->getMembers().size() == 0){
		Utils::safePrint(toStr(targetChannel->getName()) + " was removed!");
		_server.removeChannel(targetChannel->getName());
		return;
	}
	if (targetChannel->getOperators().size() == 0 && targetChannel->getMembers().size() > 0) {
        Client* oldestMember = targetChannel->getMembers().front();
        targetChannel->addOperator(oldestMember);
        std::string promotionMessage = "You have been promoted to operator in channel: " + targetChannel->getName() + "\n";
        oldestMember->sendMessage(promotionMessage);
		Utils::safePrint(toStr(oldestMember->getNick()) + " promoted to operator in channel: " + toStr(targetChannel->getName()));
    }

}


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
				Utils::safePrint(toStr(targetChannel->getName()) + " has the topic set to: " + targetChannel->getTopic());
			}
			else{
					std::string error = "You 're not an operator in " + channelName +  "\n";
					client.sendMessage(error);
			}
		}
		else{
			targetChannel->setTopic(topic);
			std::string message = "Channel topic set to: " + targetChannel->getTopic()+'\n';
			client.sendMessage(message);
			Utils::safePrint(toStr(targetChannel->getName()) + " has the topic set to: " + toStr(targetChannel->getTopic()));
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
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std:: string flag;
	stream >> flag;
	if (channelName.empty() || channelName[0] != '#') {
		std::string error = "Channel name must start with '#' or no channel name given.\n";
		client.sendMessage(error);
		return;
	}
	if(flag.empty()){
		std::string error = "You need to insert one of the appropriate flag, i, t, k, o, l.\n";
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
	if(isOperator){
		if(flag == "k"){
			std::string passWord;
			stream >> passWord;
			if(passWord.empty()){
				std::string error = "you need to add a password after the flag.\n";
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
		else if(flag == "i")
		{
			std::string mode;
			stream >> mode;
			if(mode.empty()){
				std::string error = "you need to add a mode after the flag.\n";
				client.sendMessage(error);
				return;
			}

			if(mode == "set"){
				targetChannel->setInviteStatus(true);
				std::string error = "Channel: " + channelName + " set to invitation only.\n";
				client.sendMessage(error);
			}
			else if (mode == "remove")
			{
				targetChannel->setInviteStatus(false);
				std::string error = "Invitation only in channel: " + channelName +" removed.\n";
				client.sendMessage(error);
			}

			else {
				std::string error = "You can only use set or remove.\n";
				client.sendMessage(error);
			}
		}
		else if(flag == "o"){ 
			std::string name;
			stream>>name;
			if(name.empty()){
				std::string error = "You need to insert the name of the clinet.\n";
				client.sendMessage(error);
				return ;
			}
			bool isMember = false;
			Channel* targetChannel = NULL;
			for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
				if (it->first == channelName) {
					targetChannel = it->second;
					std::vector<Client*> members = targetChannel->getMembers();
					for (std::vector<Client*>::iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt) {
						if ((*memberIt)->getNick() == name) {
							isMember = true;
							break;
						}
					}
					break;
				}
			}
			if(isMember){
				Client *clientTarget = _server.getClientByNick(name);
				std::string message = "You gave operator priviliges to: " + name +".\n";
				client.sendMessage(message);
				targetChannel->addOperator(clientTarget);
				message = "You recived operator priviliges from: " + client.getNick()+" in "+ channelName +" channel" +".\n";
				clientTarget->sendMessage(message);
				Utils::safePrint("new operator added in the channel : " + toStr(name));
			}
			else{
				std::string message = name + " not found in: " + channelName + ".\n";
				client.sendMessage(message);
			}
		}
		else if(flag == "l"){
			std::string limit;
			stream >> limit;

			if(limit.empty()){
				std::string error = "No limit given.\n";
				client.sendMessage(error);
				return;
			}
			int nb = std::atoi(limit.c_str());
			if(nb >2147483647 )
			{
				std::string error = "you can t insert a number bigger than Max_int\n";
				client.sendMessage(error);
				return ;
			}
			else if(nb < 0)
			{
				std::string error = "you can t insert a negative number\n";
				client.sendMessage(error);
				return ;
			}
			if(!isNumber(limit)){
				std::string error = "Limit must be a number.\n";
				client.sendMessage(error);
				return;
			}
			for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
				if (targetChannel->getOperators()[i] == &client) {
					isOperator = true;
					break;
				}
			}
		
			targetChannel->setLimit(nb);
			std::string message = "Limit clients in the " + channelName + " set to: " + limit + ".\n";
			client.sendMessage(message);

		}
		else if(flag == "t"){
			std::string mode;
			stream >> mode;
			if(mode.empty()){
				std::string error = "No mode given.\n";
				client.sendMessage(error);
				return; 
			}
			for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
				if (targetChannel->getOperators()[i] == &client) {
					isOperator = true;
					break;
				}
			}
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
			std::string error = "You can only use one of the appropriate flags: i,t,k,o,l\n";
			client.sendMessage(error);
		}
	}
	else{
		std::string error = "You are not an operator in " + channelName + " channel.\n";
		client.sendMessage(error);
	}
}
