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
		int serverFD;
		std::vector<struct pollfd> pollFDs;
		std::map<int, Client*> clients;
		std::map<std::string, Channel*> channels;
		Mutex clientsMutex;
		Mutex channelsMutex;
		Mutex printMutex;
		std::string const password;
		std::string const lockFilePath;
		static Server* instance;

		// Server operator credentials
		std::string _operName;
		std::string _operPassword;

		void setNonBlocking(int fd);
		void setupSignalHandlers();
		void createLockFile();
		void removeLockFile();
		void removeClient(int clientFD);
		// Disable copy constructor and assignment operator
		Server(const Server&);
		Server& operator=(const Server&);

	public:
		Server(int& port, std::string const& password);
		static void signalHandler(int signum); // does it need to be static ?
		void handleNewConnection();
		void handleClient(int clientFD);
		static Server* getInstance(); // is it the only solution?
		~Server();
		void run();
		std::string welcomeMsg();
		std::string const getPassword() const;
		std::map<std::string, Channel*>& getChannels();
		std::map<int, Client*>& getClients();
		void sendPingToClients();
		void startPingTask();
		static void* clientHandler(void* arg);
		std::string const getOperName() const;
		std::string const getOperPassword() const;
		void setOperName(void);
		void setOperPassword(void);

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