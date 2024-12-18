#include "Client.hpp"
#include "Server.hpp"
#include <unistd.h>
#include <cstring>

Client::Client(int fd) : _clientFD(fd), _authenticated(false), nickname(""), username(""), _buffer("") {}

void Client::sendMessage(const std::string &message)
{
	LockGuard lock(this->getMutex());
	std::string msg = message + "\r\n";
	ssize_t bytesSent = send(_clientFD, msg.c_str(), msg.length(), 0);
	if (bytesSent == -1)
		throw std::runtime_error("Error sending message: " + std::string(strerror(errno)));
}

Client::~Client() {}

void Client::handleRead() {
	char buffer[MAX_BUFFER];
	int nbytes = recv(_clientFD, buffer, sizeof(buffer) - 1, 0);
	if (nbytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return; // No data available
		else
			throw std::runtime_error("Error on recv: " + std::string(strerror(errno)));
	}
	else if (nbytes == 0)
	{
		throw std::runtime_error("Client disconnected: " + toStr(_clientFD));
	}
	buffer[nbytes] = '\0';
	this->_buffer += buffer;
	// printAsciiDecimal(buffer);
	// std::cout << "Message_: " << _buffer << std::endl;
	// printAsciiDecimal(buffer);

	// Process commands
	Server* server = Server::getInstance();
	{
		LockGuard lock(server->getPrintMutex()); // Use getter method
		std::cout << "Received message: " << buffer << std::endl;
	}
	CommandParser commandParser(*server);
	size_t pos;
	while ((pos = this->_buffer.find_first_of("\r\n")) != std::string::npos)
	{
		std::string command = this->_buffer.substr(0, pos);
		this->_buffer.erase(0, pos + 2); // check if 2 or 1
		// if (!this->_buffer.empty() && this->_buffer[0] == '\n') {
		// 	this->_buffer.erase(0, 1);
		// }

		std::cout << "Processing command: " << command << std::endl;
		commandParser.parseAndExecute(*this, command, server->getChannels());
	}
}

bool Client::isAuthenticated() const {
	return _authenticated;
}

void Client::setAuthenticated(bool flag) {
	_authenticated = flag;
}

void Client::setServerOperator(bool flag) {
	_serverOperator = flag;
}

int Client::getFd() const
{
	LockGuard lock(this->getMutex()); // Use getter method and const_cast
	return _clientFD;
}

bool Client::getServerOperator() const {
	return _serverOperator;
}

Mutex& Client::getMutex()
{
	return _clientMutex;
}