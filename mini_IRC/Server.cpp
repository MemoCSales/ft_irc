# include "Server.hpp"
# include "Client.hpp"


void sendErrorAndCloseFd(int fd, const std::string& response) {
	send(fd, response.c_str(), response.size(), 0);
	close(fd);
}

void sendReplyOrError(int fd, const std::string& response) {
	send(fd, response.c_str(), response.size(), 0);
}

// Function to check for the PASS command
bool checkPassCommand(const std::string& message) {
	const std::string passCommand = "PASS";
	if (message.compare(0, passCommand.length(), passCommand) == 0) {
		return true;
	}
	return false;
}

int setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		std::cerr << "fcntl(F_GETFL)" << std::endl;
		return -1;
	}

	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		std::cerr << "fcntl(F_SETFL)" << std::endl;
		return -1;
	}

	return 0;
}
