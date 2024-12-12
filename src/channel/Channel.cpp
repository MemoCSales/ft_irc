#include "Channel.hpp"
#include <algorithm>

Channel::Channel(const std::string &name) : name(name)
{
	pthread_mutex_init(&channelMutex, NULL);
}

Channel::~Channel()
{
	pthread_mutex_destroy(&channelMutex);
}

void Channel::addMember(Client *client)
{
	pthread_mutex_lock(&channelMutex);
	members.push_back(client);
	pthread_mutex_unlock(&channelMutex);
}

void Channel::removeMember(Client *client)
{
	pthread_mutex_lock(&channelMutex);
	members.erase(std::remove(members.begin(), members.end(), client), members.end());
	pthread_mutex_unlock(&channelMutex);
}

void Channel::broadcast(const std::string &message, Client *exclude)
{
	pthread_mutex_lock(&channelMutex);
	std::for_each(members.begin(), members.end(), SendMessageFunctor(exclude, message));
	pthread_mutex_unlock(&channelMutex);
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