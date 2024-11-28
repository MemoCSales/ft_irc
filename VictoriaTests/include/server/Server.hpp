#ifndef SERVER_HPP
#define SERVER_HPP

# include <vector>
# include <map>
# include <poll.h>
# include <pthread.h>
# include <netinet/in.h>
# include <arpa/inet.h> 
# include <string>
# include <cstring>

#define MAX_BUFFER 4096

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
		pthread_mutex_t clientsMutex;
		pthread_mutex_t channelsMutex;
		std::string const password;
		std::string const lockFilePath;

		void setNonBlocking(int fd);
		void handleNewConnection();
		void handleClient(int clientFD);
		void setupSignalHandlers();
		void createLockFile();
		void removeLockFile();
		// Disable copy constructor and assignment operator
		Server(const Server&);
		Server& operator=(const Server&);

	public:
		Server(int& port, std::string const& password);
		static void signalHandler(int signum); // does it need to be static ?
		~Server();
		void run();

		friend class ClientHandler;
};

class ClientHandler
{
	private:
		Server* server;
		int clientFD;

	public:
		ClientHandler(Server* srv, int fd) : server(srv), clientFD(fd) {}

		static void* start(void* arg)
		{
			ClientHandler* handler = static_cast<ClientHandler*>(arg);
			(*handler)();
			delete handler;
			return NULL;
		}

		void operator()() const
		{
			server->handleClient(clientFD);
		}
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