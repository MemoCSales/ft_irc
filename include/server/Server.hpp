#ifndef SERVER_HPP
#define SERVER_HPP

# include <vector>
# include <map>
# include <poll.h>
# include <pthread.h>
# include <netinet/in.h>
# include <arpa/inet.h> 
# include <string>
# include <iostream>
# include <cerrno>
# include <cstring> // strerror
#include "Mutex.hpp"
#include "LockGuard.hpp"
#include <csignal>

# ifndef DEBUG
#  define DEBUG 0
# endif

# define OPER_NAME "admin"
# define OPER_PASS "123"

class Client;
class Channel;


typedef std::map<int, Client*>::iterator ClientsIte;
typedef std::map<std::string, Channel*>::iterator ChannelIte;

class Server
{
	private:
		int _serverFD;
		std::string const _password;
		// bool _shutdownFlag;
		static volatile sig_atomic_t _shutdownFlag;
		// std::vector<pthread_t> workerThreads;
		// pthread_cond_t shutdownCond;
		std::vector<struct pollfd> _pollFDs;
		std::map<int, Client*> _clients;
		std::map<std::string, Channel*> _channels;
		Mutex _clientsMutex;
		Mutex _channelsMutex;
		Mutex _printMutex;
		Mutex _shutdownMutex;
		pthread_t _pingThread;
		std::string const _lockFilePath;
		static Server* _instance;

		// Server operator credentials
		std::string _operName;
		std::string _operPassword;

		void _setNonBlocking(int fd);
		void _setupSignalHandlers();
		// To delete
		void _createLockFile();
		void _removeLockFile();
		// End to delete

		void _removeClient(int clientFD);
		static void _signalHandlerWrapper(int signum);
		static void* _clientHandler(void* arg);
		static void _signalHandler(int signum); // does it need to be static ?
		void _handleNewConnection();
		void _handleClient(int clientFD);
		std::string _welcomeMsg();
		// Disable copy constructor and assignment operator
		Server(const Server&);
		Server& operator=(const Server&);

	public:
		Server(int& port, std::string const& password);
		static Server* getInstance(); // is it the only solution?
		~Server();
		void run();

		std::string const getPassword() const;
		std::map<std::string, Channel*>& getChannels();
		std::map<int, Client*>& getClients();
		// void sendPingToClients();
		// void startPingTask();
		// static void* pingTask(void* arg);

		std::string const getOperName() const;
		std::string const getOperPassword() const;
		void setOperName(void);
		void setOperPassword(void);
		Mutex& getPrintMutex(); // Add getter method for printMutex

};


/**
 * @class SockAddressInitializer
 * @brief A class to initialize and store a sockaddr_in structure.
 *
 * This class provides a convenient way to initialize a sockaddr_in structure
 * with a specified port and the loopback address (127.0.0.1).
 */

/**
 * @brief Constructor to initialize the sockaddr_in structure.
 * 
 * This constructor initializes the sockaddr_in structure with the specified
 * port number and sets the address to the loopback address (127.0.0.1).
 * 
 * @param port The port number to be set in the sockaddr_in structure.
 */

/**
 * @brief Get the initialized sockaddr_in structure.
 * 
 * This function returns the initialized sockaddr_in structure.
 * 
 * @return The initialized sockaddr_in structure.
 */
class SockAddressInitializer
{
	public:
		SockAddressInitializer(int port)
		{
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			//tp change
			inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
			addr.sin_port = htons(port);
		}

		struct sockaddr_in getAddress() const
		{
			return addr;
		}

	private:
		struct sockaddr_in addr;
};


#endif // SERVER_HPP