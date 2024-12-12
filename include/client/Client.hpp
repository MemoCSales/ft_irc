#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <pthread.h>
# include <string>
# include <Utils.hpp>
# include <cerrno>
# include <cstring> // strerror
# include <sys/socket.h>
#include <vector>
#include <Server.hpp>
#include <Channel.hpp>
# include "CommandParser.hpp"

# ifndef DEBUG
#  define DEBUG 0
# endif


# ifndef MAX_BUFFER
# define MAX_BUFFER 4096
# endif
class Server;
class Channel;

class Client 
{
	private:
		int _clientFD;
		bool _authenticated;
		Server* server;
		bool _isOperator;   //need to delete
		std::vector<Channel *> channels;
	public:
		std::string nickname;
		std::string username;
		std::string realname;
		std::string _buffer;
		pthread_t thread;

		Client( Server* srv);

		Client(int fd);
		~Client();
		int getFd() const;
		void sendMessage(const std::string &message);
		void handleRead();
		bool isAuthenticated() const;
		std::string getNick() const {return nickname;}

		// Setter
		void setAuthenticated(bool);

		//-------my fct
		int getSocket() const { return _clientFD; }
		std::string getClientNick() const {return this->nickname;}




};

// std::ostream& operator << (std::ostream& os, Client& rhs);

#endif // CLIENT_HPP