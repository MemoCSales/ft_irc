#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <pthread.h>
# include <string>
# include <Utils.hpp>
# include <cerrno>
# include <cstring> // strerror
# include <sys/socket.h>

# include "CommandParser.hpp"

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
		bool _welcomeMessage;
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
		bool hasReceiveWelcomeMessage() const;
		void checkAndSendWelcomeMessage();

		// Setter
		void setAuthenticated(bool);
		void setServerOperator(bool);
		void setReceivedWelcomeMessage(bool);

		// Getters
		bool getServerOperator() const;

};

// std::ostream& operator << (std::ostream& os, Client& rhs);

#endif // CLIENT_HPP