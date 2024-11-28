#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <vector>
# include <pthread.h>
# include "Client.hpp"

# ifndef DEBUG
#  define DEBUG 0
# endif

class Channel
{
	public:
		std::string name;
		std::vector<Client*> members;
		pthread_mutex_t mutex;

		Channel(std::string const& name);
		~Channel();
		void addMember(Client *client);
		void removeMember(Client *client);
		void broadcast(std::string const &message, Client *exclude = NULL);
};

class SendMessageFunctor
{
	private:
		Client* exclude;
		std::string message;

	public:
		SendMessageFunctor(Client* exclude, const std::string& message)
			: exclude(exclude), message(message) {}

		void operator()(Client* client) const
		{
			if (client != exclude)
			{
				client->sendMessage(message);
			}
    }
};
#endif // CHANNEL_HPP

