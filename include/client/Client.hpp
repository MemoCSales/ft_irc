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
                Channel* currentChannel; // Pointer to the current channel
                bool _isOperator;
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

		// Setter
		void setAuthenticated(bool);

                //-------my fct
                void handleCommunication(); // Handles communication with this client
                void joinChannel(const std::string& channelName,Server& server);
                void exitChanel(std::string& channelName, Server &server);
                int getSocket() const { return _clientFD; }

                ////just for  testing/debuging ///
                void	setCurrentChannel(Channel *chanel);
                // commands

                void 	chanelCommands(std::istringstream& stream);
                void 	kickCommand(const std::string& channelName, const std::string& targetName);
                void 	setPass(std::string message);
                void	modeCommands(std::string message);
                void	sendInvitation(std::string channelName);

                void	channelTopic(std::string &channelName, std::string &channelTopic);


};

// std::ostream& operator << (std::ostream& os, Client& rhs);

#endif // CLIENT_HPP