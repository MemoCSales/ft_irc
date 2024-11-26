#include "Chanel.hpp"
#include "Client.hpp"
#include <iostream>
//#include <vector>
#include <algorithm> // Ensure this header is included


void Chanel::addMember(Client* client) {
	std::lock_guard<std::mutex> lock(channelMutex);
	members.push_back(client);
}

void Chanel::removeMember(Client* client) {
	std::lock_guard<std::mutex> lock(channelMutex);
	// Use std::remove from <algorithm> to find and "move" the client to the end
	auto it = std::remove(members.begin(), members.end(), client);

	// Check if we actually found the client
	if (it != members.end()) {
		members.erase(it, members.end()); // Erase the client from the vector
	}
}

//The `Chanel::broadcast` function sends a message to all members of the channel
//except the sender. It uses a mutex to ensure thread safety while accessing the
//list of members. For each member, it checks if the member is not the sender and
//then sends the message to that member's socket.
void Chanel::broadcast(const std::string& message, Client* sender) {
	std::lock_guard<std::mutex> lock(channelMutex);
	for (Client* member : members) {
		if (member != sender) { // Don't send the message back to the sender
			send(member->getSocket(), message.c_str(), message.size() + 1, 0);
		}
	}
}