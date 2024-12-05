#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include <algorithm>

#include "Client.hpp"

class Client; // Forward declaration

class Chanel{
private:
	std::string _name; //chanel name
	std::string _passWord;
	std::string _topic;
	bool _passFlag;
	bool _created;
	std::vector<Client *> members; // clients in the chanel
	std::vector<Client *> operators;
	std::mutex channelMutex; // protects the member lists

public:
	Chanel (const std::string& channelName) : _name(channelName) {}
	void	addMember(Client* client);
	void	removeMember(Client* client);
	void	broadcast(const std::string& message, Client* sender);
	void	addOperator(Client *client);
	void	removeOperator(Client *client);
	void	broadcastTopic(Client* sender);


			//getters

	const 		std::string& getName() const { return _name; }
	bool 		getStatus() {return _created;}
	std::string getPassword(){return _passWord;}
	bool 		getPassFlag(){return _passFlag;}
	const 		std::vector<Client*>& getMembers() const { return members; } // New function to get members
	bool 		isEmpty(){return members.empty();}; // Method to check if the channel is empty
	std::string getTopic(){return _topic;}


	// setters
	void	setName(std::string chanelName){_name = chanelName ;}
	void	setStatus(bool status){_created = status;}
	void	setPassword(std::string passWord){_passWord = passWord;}
	void	setPassFlag(bool flag){_passFlag = flag;}
	void	setTopic(std::string topic){_topic = topic;}

	////just for  testing/debuging ///
	void printOperators();
	void printClients();


};

#endif