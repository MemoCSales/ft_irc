#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <algorithm> // Add this line

#include "Server.hpp"
#include "Client.hpp"



int main() {
	try {
		Server server("0.0.0.0", 54001);
		server.start();
	} catch (const std::exception& ex) {
		std::cerr << "Error: " << ex.what() << "\n";
		return 1;
	}

	return 0;
}