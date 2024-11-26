#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include "Client.hpp"

class Client; // Forward declaration

class Chanel{
private:
	std::string name; //chanel name
	std::vector<Client *> members; // clients in the chanel
	std::mutex channelMutex; // protects the member lists

public:
	explicit Chanel(const std::string& channelName) : name(channelName) {}
	void addMember(Client* client);
	void removeMember(Client* client);
	void broadcast(const std::string& message, Client* sender);
	const std::string& getName() const { return name; }
};

#endif