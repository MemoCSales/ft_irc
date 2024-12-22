# include "Commands.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"
# include "InputParser.hpp"

void sendCodeMessage(Client& client, const std::string& code, const std::string& target, const std::string& message) {
    std::string formattedMessage = ":serverhost " + code + " " + target + " :" + message;
    client.sendMessage(formattedMessage);
}


bool	modePass(std::string passWord,Client& client, std::string channelName,Channel* targetChannel){
	if(passWord.empty()){
		sendCodeMessage(client, "461", client.getNick(), "MODE :No password specified");
		return false;
	}
	if(targetChannel->getPassword() == passWord){
		sendCodeMessage(client, "901", client.getNick(), "MODE :Password already set to " + passWord);
		return false;
	}
	targetChannel->setPassword(passWord);
	sendCodeMessage(client, "902", client.getNick(), "MODE :Password for the channel: " + channelName +" set to: " +passWord);
	return true;
}

bool	modeInvite(std::string mode, Channel *targetChannel, Client &client, std::string channelName){
	if(mode.empty()){
		sendCodeMessage(client, "461", client.getNick(), "MODE :No mode specified");
		return false;
	}
	if(mode == "set"){
		if(targetChannel->getInviteStatus()){
			sendCodeMessage(client, "901", client.getNick(), "MODE :Channel already set to invitation only");
			return false;
		}
		targetChannel->setInviteStatus(true);
		sendCodeMessage(client, "902", client.getNick(), "MODE :Channel: " + channelName + " set to invitation only");

	}
	else if (mode == "remove")
	{
		if(!targetChannel->getInviteStatus()){
			sendCodeMessage(client, "901", client.getNick(), "MODE :Invitation only already removed");
			return false;
		}
		targetChannel->setInviteStatus(false);
		sendCodeMessage(client, "902", client.getNick(), "MODE :Invitation only in channel: " + channelName +" removed");
	}
	else {
		sendCodeMessage(client, "461", client.getNick(), "MODE :You can only use set or remove");
		return false; // added
	}
	return true;
}

bool modeOperator(std::string name, std::map<std::string, Channel*> &channels,Client &client, std::string channelName, Server &_server){
	if(name.empty()){
		sendCodeMessage(client, "461", client.getNick(), "MODE :No name specified");

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
			sendCodeMessage(client, "901", client.getNick(), "MODE :You can't give operator to yourself");
			return false;
		}
		bool isAlreadyOperator = false;
		std::vector<Client*> operators = targetChannel->getOperators();
		for (std::vector<Client*>::iterator It = operators.begin(); It != operators.end(); ++It) {
			if ((*It)->getNick() == name) {
				isAlreadyOperator = true;
				break;
			}
		}
		if(isAlreadyOperator)
		{
			sendCodeMessage(client, "901", client.getNick(), "MODE :User is already an operator");
			return false;
		}
		Client *clientTarget = _server.getClientByNick(name);
		sendCodeMessage(client, "902", client.getNick(), "MODE :You gave operator priviliges to: " + name);
		targetChannel->addOperator(clientTarget);
		sendCodeMessage(*clientTarget, "902", client.getNick(), "MODE :You recived operator priviliges from: " + client.getNick());
		std::cout << "new operator adde in the chanel : " <<  name << std::endl;
	}
	else{
		
		sendCodeMessage(client, "401", client.getNick(), "MODE :No such nick");
		return false;
	}
	return true;
}

bool	modeLimit(std::string limit, Client &client,Channel *targetChannel, std::string channelName){
	if(limit.empty()){
		sendCodeMessage(client, "461", client.getNick(), "MODE :No limit specified");
		return false;
	}
	if(!isNumber(limit)){
		sendCodeMessage(client, "461", client.getNick(), "MODE :Limit must be a number");
		return false;
	}

	long int nb = modAtoi(limit);
	
	if(nb >2147483647 )
	{
		sendCodeMessage(client, "901", client.getNick(), "MODE :You can't insert a number bigger than Max_int");
		return false;
	}
	else if(nb <= 0)
	{
		sendCodeMessage(client, "901", client.getNick(), "MODE :You can't insert a negative/null number");
		return false;
	}
	if ( static_cast<size_t>(nb) < targetChannel->getMembers().size()) {
		sendCodeMessage(client, "901", client.getNick(), "MODE :You can't insert a limit smaller than the number of clients already in the channel");
		return false;
	}
	if(targetChannel->getLimit() == nb){
		sendCodeMessage(client, "901", client.getNick(), "MODE :Limit already set to: " + limit);
		return false;
	}
	
	targetChannel->setLimit(nb);
	sendCodeMessage(client, "902", client.getNick(), "MODE :Limit clients in the " + channelName + " set to: " + limit);
	return true;
}

bool modeTopic(std::string mode, Client &client, Channel *targetChannel, std::string channelName){
	if(mode.empty()){
		sendCodeMessage(client, "461", client.getNick(), "MODE :No mode specified");
		return false; 
	}
	
	if(mode == "set")
	{
		if(targetChannel->getFlagTopic()){
			sendCodeMessage(client, "901", client.getNick(), "MODE :Topic restriction is already set");
			return false;
		}
		sendCodeMessage(client, "902", client.getNick(), "MODE :Topic restriction set on " + channelName);
		targetChannel->setFlagTopic(true);
	}

	else if(mode == "remove") {
		if(!targetChannel->getFlagTopic()){
			sendCodeMessage(client, "901", client.getNick(), "MODE :Topic restriction is already removed");
			return false ;
		}
		sendCodeMessage(client, "902", client.getNick(), "MODE :Topic restriction removed on " + channelName);
		targetChannel->setFlagTopic(false);
	}
	else {
		sendCodeMessage(client, "461", client.getNick(), "MODE :You can only use set or remove");
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


void sendInformativeMessage(Client& client, const std::string& baseMessage, const std::string& details = "") {
    std::string errorMessage = baseMessage;
    if (!details.empty()) {
        errorMessage += ": " + details;
    }
    errorMessage += "\n";
    client.sendMessage(errorMessage);
}




