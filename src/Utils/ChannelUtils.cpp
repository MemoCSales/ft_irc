# include "Commands.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"
# include "InputParser.hpp"

bool	modePass(std::string passWord,Client& client, std::string channelName,Channel* targetChannel){
	if(passWord.empty()){
		std::string error = "you need to add a password after the flag.\n";
		client.sendMessage(error);
		return false;
	}
	if(targetChannel->getPassword() == passWord){
		std::string error = "The password of the channel: " + channelName +" already set to " + passWord + "\n";
		client.sendMessage(error);
		return false;
	}
	targetChannel->setPassword(passWord);
	std::string error = "Password for the channel: " + channelName +" set to: " +passWord + "\n";
	client.sendMessage(error);
	return true;
}

bool	modeInvite(std::string mode, Channel *targetChannel, Client &client, std::string channelName){
	if(mode.empty()){
		std::string error = "you need to add a mode after the flag.\n";
		client.sendMessage(error);
		return false;
	}
	if(mode == "set"){
		if(targetChannel->getInviteStatus()){
			std::string error = "Channel: " + channelName + " already set to invitation only.\n";
			client.sendMessage(error);
			return false;
		}
		targetChannel->setInviteStatus(true);
		std::string error = "Channel: " + channelName + " set to invitation only.\n";
		client.sendMessage(error);
	}
	else if (mode == "remove")
	{
		if(!targetChannel->getInviteStatus()){
			std::string error = "Channel: " + channelName + " invitation only already removed.\n";
			client.sendMessage(error);
			return false;
		}
		targetChannel->setInviteStatus(false);
		std::string error = "Invitation only in channel: " + channelName +" removed.\n";
		client.sendMessage(error);
	}
	else {
		std::string error = "You can only use set or remove.\n";
		client.sendMessage(error);
		return false; // added
	}
	return true;
}

bool modeOperator(std::string name, std::map<std::string, Channel*> &channels,Client &client, std::string channelName, Server &_server){
if(name.empty()){
		std::string error = "You need to insert the name of the clinet.\n";
		client.sendMessage(error);
		return false ;
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
		if(client.getNick() == name){
			std::string error = "You are already an operator dummy.! :))\n";
			client.sendMessage(error);
			return false;
		}
		bool isAlreadyOperator;
		std::vector<Client*> operators = targetChannel->getOperators();
		for (std::vector<Client*>::iterator It = operators.begin(); It != operators.end(); ++It) {
			if ((*It)->getNick() == name) {
				isAlreadyOperator = true;
				break;
			}
		}
		if(isAlreadyOperator)
		{
			std::string error =  name + " is already an operator.!\n";
			client.sendMessage(error);
			return false;
		}
		Client *clientTarget = _server.getClientByNick(name);
		std::string message = "You gave operator priviliges to: " + name +".\n";
		client.sendMessage(message);
		targetChannel->addOperator(clientTarget);
		message = "You recived operator priviliges from: " + client.getNick()+" in "+ channelName +" channel" +".\n";
		clientTarget->sendMessage(message);
		std::cout << "new operator adde in the chanel : " <<  name << std::endl;
	}
	else{
		std::string message = name + " not found in: " + channelName + ".\n";
		client.sendMessage(message);
	}
	return true;
}

bool	modeLimit(std::string limit, Client &client,Channel *targetChannel, std::string channelName){
	if(limit.empty()){
		std::string error = "No limit given.\n";
		client.sendMessage(error);
		return false;
	}
	if(!isNumber(limit)){
		std::string error = "Limit must be a number.\n";
		client.sendMessage(error);
		return false;
	}

	long int nb = modAtoi(limit);
	
	if(nb >2147483647 )
	{
		std::string error = "you can t insert a number bigger than Max_int\n";
		client.sendMessage(error);
		return false;
	}
	else if(nb <= 0)
	{
		std::string error = "you can t insert a negative/null number\n";
		client.sendMessage(error);
		return false;
	}
	if ( static_cast<size_t>(nb) < targetChannel->getMembers().size()) {
		std::string error = "You can t insert a limit smaller than the number of clients already in the channel.\n";
		client.sendMessage(error);
		return false;
	}
	if(targetChannel->getLimit() == nb){
		std::string error = "Limit already set to: " + limit + ".\n";
		client.sendMessage(error);
		return false;
	}
	
	targetChannel->setLimit(nb);
	std::string message = "Limit clients in the " + channelName + " set to: " + limit + ".\n";
	client.sendMessage(message);
	return true;
}

bool modeTopic(std::string mode, Client &client, Channel *targetChannel, std::string channelName){
	if(mode.empty()){
		std::string error = "No mode given.\n";
		client.sendMessage(error);
		return false; 
	}
	
	if(mode == "set")
	{
		if(targetChannel->getFlagTopic()){
			std::string error = "Topic restriction is already set.\n";
			client.sendMessage(error);
			return false;
		}
		std::string error = "Topic restriction set on " + channelName + ".\n";
		client.sendMessage(error);
		targetChannel->setFlagTopic(true);
	}

	else if(mode == "remove") {
		if(!targetChannel->getFlagTopic()){
			std::string error = "Topic restriction is already removed.\n";
			client.sendMessage(error);
			return false ;
		}
		std::string error = "You removed topic restriction on " + channelName + ".\n";
		client.sendMessage(error);
		targetChannel->setFlagTopic(false);
	}
	else {
		std::string error = "You can only use set or remove.\n";
		client.sendMessage(error);
		return false;
	}
	return true;
}


bool	isMemberFct(Channel *targetChannel, Client &client, std::string channelName, std::map<std::string, Channel*> &channels){
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		if (it->first == channelName) {
			targetChannel = it->second;
			std::vector<Client*> members = targetChannel->getMembers(); // Get the members vector
			for (std::vector<Client*>::iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt) {
				if (*memberIt == &client) {
					return true;
				}
			}
			break;
		}
	}
	return false;
}

bool	isOperatorFct(Channel *targetChannel, Client &client){
	for (unsigned long int i = 0; i < targetChannel->getOperators().size(); i++){
		if (targetChannel->getOperators()[i] == &client) {
			return true;
		}
	}
	return false;
}