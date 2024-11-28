#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <cstring>
# include <unistd.h>
# include <cerrno>
# include <cstdio>
# include <sstream>
// Sockets
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <poll.h>
# include <fcntl.h>
// Container libraries
# include <vector>
# include <map>

# define IRCPORT 6667
# define PASSWORD "42"
// std::map<int, std::string> ircErrorMessages;

// void initializeIrcErrorMessages();
void sendErrorAndCloseFd(int fd, const std::string& response);
void sendReplyOrError(int fd, const std::string& response);
int setNonBlocking(int fd);

#endif
