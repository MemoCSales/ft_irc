#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <pthread.h>
# include <string>
# include <Utils.hpp>

# ifndef DEBUG
#  define DEBUG 0
# endif

class Client 
{
	public:
		int fd;
		std::string nickname;
		std::string username;
		std::string buffer;
		pthread_t thread;

		Client(int fd);
		void sendMessage(const std::string &message);
};

// std::ostream& operator << (std::ostream& os, Client& rhs);

#endif // CLIENT_HPP