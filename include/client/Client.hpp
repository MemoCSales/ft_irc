#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <pthread.h>
# include <string>
# include <Utils.hpp>
# include <cerrno>
# include <cstring> // strerror
# include <sys/socket.h>

# include "CommandParser.hpp"
#include "Mutex.hpp"
#include "LockGuard.hpp"

# ifndef DEBUG
#  define DEBUG 0
# endif


# ifndef MAX_BUFFER
# define MAX_BUFFER 4096
# endif
class Client 
{
	private:
		int _clientFD;
		bool _authenticated;
		bool _serverOperator;
		Mutex _clientMutex; // Moved to private section
	public:
		std::string nickname;
		std::string username;
		std::string realname;
		std::string _buffer;
		pthread_t thread;

		Client(int fd);
		~Client();
		int getFd() const;
		void sendMessage(const std::string &message);
		void handleRead();
		bool isAuthenticated() const;

		// Setter
		void setAuthenticated(bool);
		void setServerOperator(bool);

		// Getters
		bool getServerOperator() const;
		Mutex& getMutex(); // Added method to access clientMutex

};

// std::ostream& operator << (std::ostream& os, Client& rhs);

#endif // CLIENT_HPP