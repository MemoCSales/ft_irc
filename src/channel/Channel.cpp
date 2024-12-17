#include "Channel.hpp"
#include <algorithm>

Channel::~Channel()
{
	// pthread_mutex_destroy(&channelMutex);
}

void Channel::addMember(Client *client)
{
	// pthread_mutex_lock(&channelMutex);
	members.push_back(client);
	// pthread_mutex_unlock(&channelMutex);
}
void Channel::addOperator(Client *client) {
	// std::lock_guard<std::mutex> lock(channelMutex);
	operators.push_back(client);
}

void Channel::addAllowedPeople(Client *client) {
	people.push_back(client);
}

void Channel::removeMember(Client *client) {
	// std::lock_guard<std::mutex> lock(channelMutex);
	// Use std::remove from <algorithm> to find and "move" the client to the end
	std::vector<Client *>::iterator it = std::remove(members.begin(), members.end(), client);
	// Check if we actually found the client
	if (it != members.end()) {
		members.erase(it, members.end());// Erase the client from the vector
//		client->setCurrentChannel(nullptr);
	}
}


void Channel::removeOperator(Client *client) {
	// std::lock_guard<std::mutex> lock(channelMutex);
	std::vector<Client *>::iterator it = std::remove(operators.begin(), operators.end(), client);
	if (it != operators.end()) {
		operators.erase(it, operators.end());
	}
}

void Channel::broadcast(const std::string &message, Client *sender) {
	// std::lock_guard<std::mutex> lock(channelMutex);
	std::string messageWithSender = "[" + _name + "][" + sender->getNick() + "] " + message + "\n";

	for (std::vector<Client *>::iterator it = members.begin(); it != members.end(); ++it) {
		Client *member = *it;
		if (member != sender) {
			send(member->getSocket(), messageWithSender.c_str(), messageWithSender.size() + 1, 0);
		}
	}
}

void	Channel::broadcastTopic( Client* sender){
  std::string message = "The topic of the channel is: " + this->getTopic() + "\n";
  send(sender->getSocket(), message.c_str(), message.size() + 1, 0);
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
