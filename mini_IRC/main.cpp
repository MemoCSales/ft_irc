# include "Server.hpp"
# include "Client.hpp"

int main() {

	// Create socket
	int serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd < 0) {
		std::cerr << "Error creating server socket." << std::endl;
		return 1;
	}
	std::cout << "Socket created successfully" << std::endl;

	// Set socket options
	int opt = 1;
	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		std::cerr << "Error in socket options" << std::endl;
		close(serverFd);
		return 1;
	}
	
	setNonBlocking(serverFd);

	// Assign socket to port (bind)
	struct sockaddr_in address;
	std::memset(&address, 0, sizeof(address));

	address.sin_family = AF_INET;
	// address.sin_addr.s_addr = INADDR_ANY;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(IRCPORT);

	if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		std::cerr << "Error to find socket to port." << std::endl;
		close(serverFd);
		return 1;
	}

	// Listening to the assigned socket
	if (listen(serverFd, 5) < 0) {
		std::cerr << "Error in listening to the assigned socket" << std::endl;
		close(serverFd);
		return 1;
	}

	std::cout << "Server listening from port " << IRCPORT << "..." << std::endl;


	// Creating a list of sockets for poll()
	std::vector<struct pollfd> pollFds;

	//Add server socket to vector
	struct pollfd serverPollFd = {serverFd, POLLIN, 0};
	pollFds.push_back(serverPollFd);

	//main while loop
	while (true) {
		// Call poll to detect events
		int pollCount = poll(pollFds.data(), pollFds.size(), -1);	// Waits undefinedtly haha
		if (pollCount < 0) {
			std::cerr << "Error in poll()" << std::endl;
			break;
		}

		// Loop through sockets to process events
		for (size_t i = 0; i < pollFds.size(); i++) {
			if (pollFds[i].revents & POLLIN) {
				// If server socket -> accept new connection
				if (pollFds[i].fd == serverFd) {
					int clientFd = accept(serverFd, NULL, NULL);
					if (clientFd < 0) {
						perror("Error to accept connection");
						continue;
					}

					// Configure new non blocking socket
					setNonBlocking(clientFd);

					Client* client = new Client(clientFd, PASSWORD);
					connections[clientFd] = client;

					// Add new client to poll() list
					struct pollfd clientPollFd = {clientFd, POLLIN, 0};
					pollFds.push_back(clientPollFd);

					std::cout << "New client connected -> fd: " << clientFd << std::endl;
				}
				// If client -> read data
				else {
					int clientFd = pollFds[i].fd;
					try
					{
						connections[clientFd]->handleRead();
					} catch(const std::runtime_error& e) {
						std::cerr << e.what() << '\n';
						close(clientFd);
						delete connections[clientFd];
						connections.erase(clientFd);
						pollFds.erase(pollFds.begin() + i);
						--i;
					}
				}
			}
		}
	}

	close(serverFd);

	return 0;
}
