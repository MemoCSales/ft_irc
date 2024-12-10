#ifndef NUMERIC_MESSAGES_HPP
# define NUMERIC_MESSAGES_HPP


# define USERLEN 30

/* ERROR MESSAGES */
# define ERR_PASSWDMISMATCH		 ":server 464 * :Password incorrect\r\n"
# define ERR_NICKNAMEINUSE(nick) ":server 433 * :" + nick + ":Nickname is already in use\r\n"
# define ERR_UNKNOWNCOMMAND(command) ":server 421 * " + command + ":Unknown command\r\n"
# define ERR_NEEDMOREPARAMS(user) ":server 461 * " + user + " :Not enough parameters\r\n"
# define ERR_ALREADYREGISTERED ":server 462 * :You may not reregister\r\n"
# define ERROR(reason) "Error: " + reason + "\r\n"
# define ERR_NOOPERHOST " server 491 * :No O-lines for your host\r\n"
# define ERR_NOTREGISTERED " server 451 * :You have not registered\r\n"


/* REPLY MESSAGES */
# define RPL_NICKCHANGE(oldNick, user, host, newNick)  ":" + oldNick + "!" + user + "@" + host + " NICK :" + newNick + "\r\n"
# define RPL_EMPTYCAPLIST(host) ":" + host + " CAP " + " LS :\r\n"
# define RPL_QUIT(reason) "Quit" + reason + "\r\n"
# define RPL_WELCOME(nickname) "Welcome to our IRC network, " + nickname + " !\r\n"
# define RPL_YOUREOPER "You are now an IRC server operator \r\n"

#endif
