#include "Client.hpp"
#include <unistd.h>
#include <cstring>

Client::Client(int fd) : fd(fd), nickname(""), username(""), buffer("") {}

void Client::sendMessage(const std::string &message)
{
	write(fd, message.c_str(), message.length());
}
