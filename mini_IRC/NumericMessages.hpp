#ifndef NUMERIC_MESSAGES_HPP
# define NUMERIC_MESSAGES_HPP


# define USERLEN 30

/* ERROR MESSAGES */
# define ERR_PASSWDMISMATCH		 ":server 464 * :Password incorrect\r\n"
# define ERR_NICKNAMEINUSE(nick) ":server 433 * :" + nick + ":Nickname is already in use\r\n"
# define ERR_UNKNOWNCOMMAND(command) ":server 421 * " + command + ":Unknown command\r\n"
# define ERR_NEEDMOREPARAMS ":server 461 * USER :Not enough parameters\r\n"
# define ERR_ALREADYREGISTERED ":server 462 * :You may not reregister\r\n"
# define ERROR(reason) "Error :" + reason + "\r\n"


/* REPLY MESSAGES */
# define RPL_NICKCHANGE(oldNick, user, host, newNick)  ":" + oldNick + "!" + user + "@" + host + " NICK :" + newNick + "\r\n"
# define RPL_EMPTYCAPLIST(host) ":" + host + " CAP " + " LS :\r\n"
# define RPL_QUIT(reason) "Quit" + reason + "\r\n"

#endif
