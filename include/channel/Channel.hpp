#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <string>
#include <algorithm>

#include "Client.hpp"
#define  MAX 2147483647

class Client; // Forward declaration

class Channel{
private:
	std::string _name; //chanel name
	std::string _passWord;
	std::string _topic;
	bool _inviteOnly;
	int _limit;
	bool _flagTopic;
	std::vector<Client *> members; // clients in the chanel
	std::vector<Client *> operators;
	std::vector<Client *> people; 


public:

	Channel (const std::string& channelName) : _name(channelName), _passWord(""), _topic(""),_inviteOnly(false), _limit(MAX),_flagTopic(false) {}
	void	addMember(Client* client);
	void	removeMember(Client* client);
	void	broadcast(const std::string& message, Client* sender);
	void	addOperator(Client *client);
	void	addAllowedPeople(Client *client);
	void	removeOperator(Client *client);
	void	broadcastTopic(Client* sender);
	bool 	isMember(Client *client);
	~Channel();

			//getters

	const 		std::string& getName() const { return _name; }
	bool 		getFlagTopic() {return _flagTopic;}
	std::string getPassword(){return _passWord;}
	const 		std::vector<Client*>& getMembers() const { return members; } // New function to get members
	const 		std::vector<Client*>& getOperators() const { return operators; } // New function to get members
	const 		std::vector<Client*>& getAllowedPeople() const { return people; } // New function to get members
	bool 		isEmpty(){return members.empty();}; // Method to check if the channel is empty
	bool		getInviteStatus(){return _inviteOnly;}
	std::string getTopic(){return _topic;}
	int 		getLimit(){return _limit;}

	// setters
	void	setName(std::string chanelName){_name = chanelName ;}
	void	setFlagTopic(bool status){_flagTopic = status;}
	void	setPassword(std::string passWord){_passWord = passWord;}
	void	setTopic(std::string topic){_topic = topic;}
	void	setInviteStatus(bool flag){_inviteOnly = flag;}
	void	setLimit(int nb){_limit = nb;}




};

// class SendMessageFunctor
// {
// 	private:
// 		Client* exclude;
// 		std::string message;

// 	public:
// 		SendMessageFunctor(Client* exclude, const std::string& message)
// 			: exclude(exclude), message(message) {}

// 		void operator()(Client* client) const
// 		{
// 			if (client != exclude)
// 			{
// 				client->sendMessage(message);
// 			}
//     }
// };


#endif