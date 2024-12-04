#include "Client.hpp"
#include <unistd.h>
#include <cstring>

Client::Client(int fd) : _clientFD(fd), nickname(""), username(""), buffer("") {}

void Client::sendMessage(const std::string &message)
{
	write(_clientFD, message.c_str(), message.length());
}

Client::~Client() {}

void Client::handleRead() {
	char buffer[MAX_BUFFER];
	int nbytes = recv(_clientFD, buffer, sizeof(buffer) - 1, 0);
	if (nbytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return; // No data available
		else
			throw std::runtime_error("Error on recv: " + std::string(strerror(errno)));
	} else if (nbytes == 0) {
		throw std::runtime_error("Client disconnected");
	}
	buffer[nbytes] = '\0';
	this->buffer += buffer;

	// Process commands
	size_t pos;
	while ((pos = this->buffer.find("\r\n")) != std::string::npos)
	{
		std::string command = this->buffer.substr(0, pos);
		this->buffer.erase(0, pos + 2);
		std::cout << "Received command from " << _clientFD << ": " << command << std::endl;
		// Handle command
	}
}

int Client::getFd() const
{
	return _clientFD;
}