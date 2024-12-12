#include "Channel.hpp"
#include <algorithm>



//pthread_mutex_lock(&mutex);

//-------------------my functions ---------------

void Channel::addMember(Client *client) {
	// std::lock_guard<std::mutex> lock(channelMutex);
	members.push_back(client);
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

//The `Chanel::broadcast` function sends a message to all members of the channel
//except the sender. It uses a mutex to ensure thread safety while accessing the
//list of members. For each member, it checks if the member is not the sender and
//then sends the message to that member's socket.


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
  // std::lock_guard<std::mutex> lock(channelMutex);
//  std::cout << "entered in brodcast topic\n";
  std::string message = "The topic of the channel is: " + this->getTopic() + "\n";
  send(sender->getSocket(), message.c_str(), message.size() + 1, 0);
}

/// ------------------testing---------------/////

void Channel::printOperators() {
	// std::lock_guard<std::mutex> lock(channelMutex);
	std::cout << "Operators in channel " << _name << std::endl;
	for (long unsigned int i = 0; i < operators.size(); i++) {
		std::cout << operators[i] << " ";
	}
	std::cout << "\n";
}

void Channel::printClients() {
	// std::lock_guard<std::mutex> lock(channelMutex);
	std::cout << "Clients in channel " << _name << std::endl;
	for (long unsigned int i = 0; i < members.size(); i++) {
		std::cout << members[i] << " ";
	}
	std::cout << "\n";
}