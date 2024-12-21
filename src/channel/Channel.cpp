#include "Channel.hpp"
#include <algorithm>

Channel::~Channel()
{
}

void Channel::addMember(Client *client)
{
	members.push_back(client);
}
void Channel::addOperator(Client *client) {
	operators.push_back(client);
}

void Channel::addAllowedPeople(Client *client) {
	people.push_back(client);
}

void Channel::removeMember(Client *client) {
	
	std::vector<Client *>::iterator it = std::remove(members.begin(), members.end(), client);
	if (it != members.end()) {
		members.erase(it, members.end());
	}
}


void Channel::removeOperator(Client *client) {
	std::vector<Client *>::iterator it = std::remove(operators.begin(), operators.end(), client);
	if (it != operators.end()) {
		operators.erase(it, operators.end());
	}
}

void Channel::removePeople(Client *client) {
	std::vector<Client *>::iterator it = std::remove(people.begin(), people.end(), client);
	if (it != people.end()) {
		people.erase(it, people.end());
	}
}

void Channel::broadcast(const std::string &message, Client *sender) {
	std::string messageWithSender = ":" + sender->getNick() + "!" + sender->username + "@localhost PRIVMSG " + this->getName() + " :" + message;

	for (std::vector<Client *>::iterator it = members.begin(); it != members.end(); ++it) {
		Client *member = *it;
		if (member != sender) {
			member->sendMessage(messageWithSender);
		}
	}
}

void	Channel::broadcastTopic( Client* sender){
	for (std::vector<Client *>::iterator it = members.begin(); it != members.end(); ++it) {
		Client *member = *it;
		if (member != sender) {
			member->sendMessage(":serverhost 332 " + member->getNick() + " " + this->getName() + " :" + this->getTopic());

		}
	}
}

void	Channel::broadcastClientState( Client* client,std::string state){
	std::string message;
	for (std::vector<Client *>::iterator it = members.begin(); it != members.end(); ++it) {
		Client *member = *it;
		if (member != client) {
			if(state == "join")
				message = ":" + client->getNick() + "!user@host JOIN :" + this->getName();
			else
				message = ":" + client->getNick() + "!user@host PART :" + this->getName();
			member->sendMessage(message);
		}
	}
}


void	Channel::sendUsersList(Client *client){

	for (std::vector<Client *>::iterator it = members.begin(); it != members.end(); ++it) {
		Client *member = *it;
		std::string message = ":serverhost 353 " + client->getNick() + " = " + this->getName() + " :";
		if (member != client) {
			if(this->isOperator(member))
				message += "@" + member->getNick() + " ";
			else
				message += member->getNick() + " ";
			client->sendMessage(message);
		}
	}
	client->sendMessage(":serverhost 366 " + client->getNick() + " " + this->getName() + " :End of /NAMES list");
}


bool Channel::isMember(Client *client) {
	bool found = false;
	for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); it++) {
		if ((*it)->nickname == client->nickname) {
			found = true;
			return found;
		}
	}
	return found;
}

bool Channel::isOperator(Client *client) {
	bool found = false;
	for (std::vector<Client*>::iterator it = operators.begin(); it != operators.end(); it++) {
		if ((*it)->nickname == client->nickname) {
			found = true;
			return found;
		}
	}
	return found;
}

bool Channel::isInvited(Client *client) {
	bool found = false;
	for (std::vector<Client*>::iterator it = people.begin(); it != people.end(); it++) {
		if ((*it)->nickname == client->nickname) {
			found = true;
			return found;
		}
	}
	return found;
}

// ----------------------test-----------------
void Channel::setTopic(const std::string& newTopic, const std::string& setter) {
    _topic = newTopic;
    _topicSetter = setter;
    _topicTimestamp = std::time(NULL); // Get current timestamp
}


std::string Channel::getTopicSetter() const {
    return _topicSetter;
}

std::time_t Channel::getTopicTimestamp() const {
    return _topicTimestamp;
}
