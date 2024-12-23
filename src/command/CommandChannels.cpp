# include "Commands.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"
# include "InputParser.hpp"
#include "ChannelUtils.hpp"

void Command::handleJoin(Client& client, const std::string& args, std::map<std::string, Channel*>& channels) {
	std::string channelName = trim(args);
	// Check if client registration is complete before joining a channel
	if (!client.isRegistered()) {
		client.sendMessage("Complete your registration before joining a channel");
		return;
	}
	if (channelName.empty()) {
		sendInformativeMessage(client,"Error: No channel name provided.","");
		return;
	}
	if(channelName[0] != '#'){
		sendInformativeMessage(client,"Error: Name of the channel must start with '#'.","");
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

	if (!found) {  //maybe create a function for this
		targetChannel = _server.getOrCreateChannel(channelName);
		targetChannel->addMember(&client);
		targetChannel->addOperator(&client);
		client.sendMessage(":" + client.getNick() + "!" + client.username + "@localhost" + " JOIN " + channelName + "\r\n");
		targetChannel->sendUsersList(&client);
		targetChannel->broadcastClientState(&client,"join");
		Utils::safePrint(client.getNick() + " has joined the channel: " + toStr(targetChannel->getName()));
		if (!targetChannel->getTopic().empty()) {
			client.sendMessage(":serverhost 332 " + client.getNick() + " " + channelName + " :" + targetChannel->getTopic());
			std::stringstream ss;
			ss << targetChannel->getTopicTimestamp();
			std::string topicTimestampStr = ss.str();
			// client.sendMessage(":serverhost 333 " + client.getNick() + " " + channelName + " " + targetChannel->getTopicSetter() + " " + std::to_string(targetChannel->getTopicTimestamp()));
			client.sendMessage(":serverhost 333 " + client.getNick() + " " + channelName + " " + targetChannel->getTopicSetter() + " " + topicTimestampStr);
	} else {
		client.sendMessage(":serverhost 332 " + client.getNick() + " " + channelName + " :\r\n");
	}
		// sendInformativeMessage(client,channelName + " was created","you are the operator of this channel.");
		Utils::safePrint(toStr(channelName) + " was created!");
		return;
	} else {
		// Check if the client is in channel;
		if(isMemberFct(targetChannel,client,channelName,channels)){
			sendInformativeMessage(client,"Already in the channel.","");
			return;
		}
	
		// check if chaennel is full
		if (targetChannel->getMembers().size() >= static_cast<size_t>(targetChannel->getLimit())) {
			sendInformativeMessage(client,"Channel is full. Unable to join.","");
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
				sendInformativeMessage(client,"This is a private channel, you need an invitation.","");
				return;
			}
		}
		if (!targetChannel->getPassword().empty()) {
			sendInformativeMessage(client,"Private channel, requires a password.","");
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
						sendInformativeMessage(client,"Joined Channel",targetChannel->getName());
						targetChannel->sendUsersList(&client);
						targetChannel->broadcastClientState(&client,"join");
						Utils::safePrint(client.getNick() + " has joined the channel: " + toStr(targetChannel->getName()));
						if (!targetChannel->getTopic().empty())
							sendInformativeMessage(client,"The topic of the channel" + channelName + "is",targetChannel->getTopic());
							// targetChannel->broadcastTopic(&client);
						return;
					} else {
						sendInformativeMessage(client,"Incorrect password","");
						return;
					}
				}
			}
		}
	}
	// Join the channel	
	targetChannel->addMember(&client);
	// sendInformativeMessage(client,"Joined Channel",targetChannel->getName());
	client.sendMessage(":" + client.getNick() + "!" + client.username + "@localhost" + " JOIN " + channelName + "\r\n");
	targetChannel->sendUsersList(&client);
	targetChannel->broadcastClientState(&client,"join");
	Utils::safePrint(client.getNick() + " has joined the channel: " + toStr(targetChannel->getName()));
	if (!targetChannel->getTopic().empty()) {
   	 	client.sendMessage(":serverhost 332 " + client.getNick() + " " + channelName + " :" + targetChannel->getTopic());
		std::stringstream ss;
		ss << targetChannel->getTopicTimestamp();
		std::string topicTimestampStr = ss.str();
    	// client.sendMessage(":serverhost 333 " + client.getNick() + " " + channelName + " " + targetChannel->getTopicSetter() + " " + std::to_string(targetChannel->getTopicTimestamp()));
		client.sendMessage(":serverhost 333 " + client.getNick() + " " + channelName + " " + targetChannel->getTopicSetter() + " " + topicTimestampStr);
	} else {
		client.sendMessage(":serverhost 332 " + client.getNick() + " " + channelName + " :\r\n");
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
		sendInformativeMessage(client,"Channel name must start with '#' or channel is empty.","");
		return;
	}
	if (target.empty()) {
		sendInformativeMessage(client,"You need to choose a client to kick.","");
		return;
	}

	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		sendInformativeMessage(client,"Channel does not exist.","");
		return;
	}
	bool isOperator = isOperatorFct(targetChannel,client);
	if (isOperator){
		bool found = false;
		for (unsigned long int i = 0; i < targetChannel->getMembers().size(); i++){
			if (targetChannel->getMembers()[i]->getNick() == target) {
				found = true;
				if(target == client.getNick()){
					sendInformativeMessage(client,"You can t kick yourself dummy :))!. Use PART <#channel name> to exit channel.","");
					return;
				}
				targetChannel->broadcastClientState(targetChannel->getMembers()[i], "kick");
				targetChannel->removeMember(targetChannel->getMembers()[i]);

				// Update NAMES list
				for (std::vector<Client*>::const_iterator it = targetChannel->getMembers().begin(); 
				it != targetChannel->getMembers().end(); ++it) {
				targetChannel->sendUsersList(*it);
			}
				// std::string response = "You got kicked from the channel:" + channelName + "\n";
				// send(targetChannel->getMembers()[i]->getFd(), response.c_str(), response.size() + 1, 0);
				Utils::safePrint(toStr(targetChannel->getMembers()[i]->getNick()) + " got kicked from: " + toStr(targetChannel->getName()));
				break;
			}
		}
		if (!found){
			sendInformativeMessage(client,"User not found in the channel",channelName);
		}
	}
	else {
		sendInformativeMessage(client,"You 're not an operator in",channelName);
	}
}

void	Command::handlePart(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	if (channelName.empty() || channelName[0] != '#') {
		sendInformativeMessage(client,"Channel name must start with '#' or channel is empty", "");
		return;
	}
	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		sendInformativeMessage(client ,"Channel does not exist","");
		return;
	}
	bool isOperator = isOperatorFct(targetChannel,client);
	bool found = false;
	for (unsigned long int i = 0; i < targetChannel->getMembers().size(); i++){
		if (targetChannel->getMembers()[i] == &client) {
			found = true;
			if (found) {
				targetChannel->broadcastClientState(&client, "part");

				// Send explicit part message to the leaving client
				client.sendMessage(":" + client.getNick() + "!" + client.username + "@localhost PART " + channelName + "\r\n");

				if(isOperator)	{
					targetChannel->removeOperator(&client);
				}
				targetChannel->removeMember(&client);
				// sendInformativeMessage(client,"Exited channel", targetChannel->getName());
			}
			Utils::safePrint(client.getNick() + " has joined the channel: " + toStr(targetChannel->getName()));
			break;
		}
	}
	if (!found){
		sendInformativeMessage(client,"User are not part of that channel","");
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
		sendInformativeMessage(client,"Channel name must start with '#' or channel is empty", "");
		return;
	}
	if(topic.empty()){
		sendInformativeMessage(client,"Topic can't be empty.","");
		return;
	}
	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		sendInformativeMessage(client,"Channel does not exist","");
		return;
	}
	bool isMember = isMemberFct(targetChannel,client,channelName, channels);
	if(isMember){
		if(targetChannel->getFlagTopic() == true){
			sendInformativeMessage(client,"Topic restriction set to true, You need to be an operator.","");
			bool isOperator = isOperatorFct(targetChannel,client);
			if(isOperator){
				if(targetChannel->getTopic() == topic){
					sendInformativeMessage(client,"Topic already set to",topic);
					return;
				}
				targetChannel->setTopic(topic,client.getNick());
				std::string topicMsg = RPL_TOPIC(client.getNick(), client.username, targetChannel->getName(), targetChannel->getTopic());
				client.sendMessage(topicMsg);
				targetChannel->broadcastTopic(&client);
				Utils::safePrint(toStr(targetChannel->getName()) + " has the topic set to: " + targetChannel->getTopic());
			}
			else{
				std::string errorMsg = RPL_PRIVMSG(client.getNick(), client.username, channelName, ERR_CHANOPRIVSNEEDED(channelName));
				client.sendMessage(errorMsg);
				return ;
			}
		}
		else{
			if(targetChannel->getTopic() == topic){
					sendInformativeMessage(client, "Topic already set to", topic);
					return;
				}
			targetChannel->setTopic(topic,client.getNick());
			std::string topicMsg = RPL_TOPIC(client.getNick(), client.username, targetChannel->getName(), targetChannel->getTopic());
			client.sendMessage(topicMsg);
			targetChannel->broadcastTopic(&client);
			Utils::safePrint(toStr(targetChannel->getName()) + " has the topic set to: " + targetChannel->getTopic());
		}
	}
	else{
		sendInformativeMessage(client,"You are not a member of this channel","");
		return ;
	}
	
}
		

void	Command::handleInvite(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	(void)channels;
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std::string targetNick;
	stream >> targetNick;
	if (channelName.empty() || channelName[0] != '#') {
		sendInformativeMessage(client,"Channel name must start with '#' or channel is empty", "");
		return;
	}
	if(targetNick.empty()){
		sendInformativeMessage(client,"no nickname given.","");
		return;
	}

	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		sendInformativeMessage(client,"Channel does not exist.","");
		return;
	}
	bool isOperator = isOperatorFct(targetChannel, client);
	bool found = false;
	for (unsigned long int i = 0; i < targetChannel->getMembers().size(); i++){
		if (targetChannel->getMembers()[i] == &client) {
			found = true;
			if(isOperator){
				Client *targetClient = _server.getClientByNick(targetNick);
				bool isMember = isMemberFct(targetChannel,*targetClient,channelName, channels);
				if(isMember){
					std::string response = targetNick + " already in the channel.\n";
					client.sendMessage(response);
					return ;
				}
				if (targetClient) {
					bool invitedAlready = false;
					std::vector<Client*> invited = targetChannel->getAllowedPeople();
					for (std::vector<Client*>::iterator It = invited.begin(); It != invited.end(); ++It) {
						if ((*It)->getNick() == targetNick) {
							invitedAlready = true;
							break;
						}
					}
					if(invitedAlready){
						sendInformativeMessage(client,targetNick + " already invited in the", channelName);
						return ;
					}
					targetChannel->addAllowedPeople(targetClient);
					std::string response = "You have been invited to join channel: " + channelName + "\n";
					targetClient->sendMessage(response);
				} else {
					sendInformativeMessage(client,"User with nickname", targetNick + " not found");
				}
			} else {
					sendInformativeMessage(client,"You 're not an operator in", targetChannel->getName());
			}
			break;
		}
	}
	if (!found){
		sendInformativeMessage(client,"You are not part of",  targetChannel->getName() + " channel");
	}
}

void	Command::handleMode(Client& client, const std::string& args, std::map<std::string, Channel*>& channels){
	std::istringstream stream(args);
	std::string channelName;
	stream >> channelName;
	std:: string flag;
	stream >> flag;
	if (channelName.empty() || channelName[0] != '#') {
		sendInformativeMessage(client,"Channel name must start with '#' or channel is empty", "");
		return;
	}
	Channel *targetChannel = _server.getChannel(channelName);
	if (!targetChannel) {
		sendInformativeMessage(client,"Channel", channelName + " does not exist.");
		return;
	}
	if(flag.empty()){
		sendInformativeMessage(client, ":" + client.getNick() + "!" + client.username + "@localhost PRIVMSG " + targetChannel->getName() + " :" + "You can only use one of the appropriate flags: i,t,k,o,l.","");
		return;
	}

	bool isOperator = isOperatorFct(targetChannel,client);
	if(isOperator){
		if(flag == "k"){
			std::string passWord;
			stream >> passWord;
			if(!modePass(passWord,client,channelName,targetChannel)) return ;
		}
		else if(flag == "i")
		{
			std::string mode;
			stream >> mode;
			if(!modeInvite(mode,targetChannel,client,channelName)) return ;
		}
		else if(flag == "o"){ 
			std::string name;
			stream>>name;
			if(!modeOperator(name,channels,client,channelName,_server)) return ;
		}
		else if(flag == "l"){
			std::string limit;
			stream >> limit;
			if(!modeLimit(limit,client, targetChannel,channelName)) return ;
		}
		else if(flag == "t"){
			std::string mode;
			stream >> mode;
			if(!modeTopic(mode,client,targetChannel,channelName)) return ;
		}
		else{
			std::string messageWithSender = "You can only use one of the appropriate flags: i,t,k,o,l.";
			client.sendMessage(RPL_PRIVMSG(client.getNick(), client.username, targetChannel->getName(), messageWithSender));
		}
	}
	else{
		sendInformativeMessage(client,"You are not an operator in",channelName +" channel.");
	}
}




//topic broadcast reversed -->done
//broadcast client join -->done
// broadcast client part -->done
//provide list with members in channel-->


// need to do the same when signal killed'
