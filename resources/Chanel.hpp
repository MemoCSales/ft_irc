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
	bool _passFlag;
	bool _created;
	std::vector<Client *> members; // clients in the chanel
	std::vector<Client *> operators;
	std::mutex channelMutex; // protects the member lists

public:
	Chanel (const std::string& channelName) : _name(channelName), _passFlag(0) {}
	void	addMember(Client* client);
	void	removeMember(Client* client);
	void	broadcast(const std::string& message, Client* sender);
	void	addOperator(Client *client);

			//getters

	const 		std::string& getName() const { return _name; }
	bool 		getStatus() {return _created;}
	std::string getPassword(){return _passWord;}
	bool 		getPassFlag(){return _passFlag;}


			// setters
	void	setName(std::string chanelName){_name = chanelName ;}
	void	setStatus(bool status){_created = status;}
	void	setPassword(std::string passWord){_passWord = passWord;}
	void	setPassFlag(bool flag){_passFlag = flag;}

	////just for  testing/debuging ///
	void printOperators();
	void printClients();


};

#endif