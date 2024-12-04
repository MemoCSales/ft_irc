#include "Channel.hpp"
#include <algorithm>

Channel::Channel(const std::string &name) : name(name)
{
	pthread_mutex_init(&mutex, NULL);
}

Channel::~Channel()
{
	pthread_mutex_destroy(&mutex);
}

void Channel::addMember(Client *client)
{
	pthread_mutex_lock(&mutex);
	members.push_back(client);
	pthread_mutex_unlock(&mutex);
}

void Channel::removeMember(Client *client)
{
	pthread_mutex_lock(&mutex);
	members.erase(std::remove(members.begin(), members.end(), client), members.end());
	pthread_mutex_unlock(&mutex);
}

void Channel::broadcast(const std::string &message, Client *exclude)
{
	pthread_mutex_lock(&mutex);
	std::for_each(members.begin(), members.end(), SendMessageFunctor(exclude, message));
	pthread_mutex_unlock(&mutex);
}