#include "Command.hpp"
#include <sstream>
#include <iostream>

void Command::handleCommand(const std::string &command, Client *client, std::map<std::string, Channel*> &channels)
{
	std::istringstream iss(command);
	std::string cmd;
	iss >> cmd;

	if (cmd == "NICK")
	{
		iss >> client->nickname;
	}
	else if (cmd == "USER")
	{
		iss >> client->username;
	}
	else if (cmd == "JOIN")
	{
		std::string channelName;
		iss >> channelName;
		if (channels.find(channelName) == channels.end())
		{
			channels.insert(std::make_pair(channelName, new Channel(channelName)));
		}
		channels[channelName]->addMember(client);
		channels[channelName]->broadcast(client->nickname + " has joined the channel.\n", client);
	}
	else if (cmd == "PRIVMSG")
	{
		std::string target, message;
		iss >> target;
		std::getline(iss, message);
		if (channels.find(target) != channels.end())
		{
			channels[target]->broadcast(client->nickname + ": " + message + "\n", client);
		}
	}
}