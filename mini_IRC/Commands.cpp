# include "Commands.hpp"
# include "NumericMessages.hpp"
# include "Utils.hpp"


void Command::execute(Client& client, const std::string& args) {
	switch (_type)
	{
	case CAP:
		{	
			std::cout << "Args in CAP: " << args << std::endl;
			std::string subcommand = Utils::trim(args);
			std::cout << "subcommand: " << subcommand << std::endl;
			if (subcommand == "LS 302") {
				std::string host = "localhost";
				// std::string nick = client.getNick();
				std::string response = RPL_EMPTYCAPLIST(host);
				std::cout << response << std::endl;
				sendReplyOrError(client.getFd(), response);
				client.setCapNegotiation(true);
				std::cout << "Client AUTH in execute: " << client.isAuthenticated() << std::endl;
				std::cout << "Client CAP Negotiation in execute: " << client.isCapNegotiation() << std::endl;
			}
			if (subcommand == "END" && client.isCapNegotiation()) {
				client.setCapNegotiation(false);
				std::cout << "Client AUTH in execute: " << client.isAuthenticated() << std::endl;
				std::cout << "Client CAP Negotiation in execute: " << client.isCapNegotiation() << std::endl;
			}
		}
		break;
	case PASS:
		{
			std::string password = Utils::trim(args);
			if (password == client.getCorrectPassword()) {
				client.setAuthenticated(true);
				std::cout << "Client authenticated -> fd: " << client.getFd() << std::endl;
			} else {
				std::string response = ERR_PASSWDMISMATCH;
				// std::cout << response << std::endl;	
				sendErrorAndCloseFd(client.getFd(), response);
			}
		}
		break;
	case NICK:
		{
			std::string newNick = Utils::trim(args);
			std::string oldNick = client.getNick();
			bool nickExists = false;

			//check for a valid newNickname : ERR_ERRONEUSNICKNAME 432
			std::map<int, Client*>::iterator it = connections.begin();
			for (; it != connections.end(); it++) {
				if (it->second->getNick() == newNick) {
					nickExists = true;
					break;
				}
			}
			if (nickExists) {
				std::string response = ERR_NICKNAMEINUSE(newNick);
				sendErrorAndCloseFd(client.getFd(), response);
			} else {
				client.setNick(newNick);
				std::string user = client.getUser();
				std::string host = "localhost";

				// Acknowledge the NICK command was successfull and print the new nick
				std::string response = RPL_NICKCHANGE(oldNick, user, host, newNick);
				sendReplyOrError(client.getFd(), response);

				// todo: Inform other clients about the nickname change
			}
			std::cout << "NICK command received. Client nickname changed from: " << oldNick << " to: " << newNick << std::endl;
		}
		break;
	case USER:
		{
			std::istringstream stream(args);
			std::string userName, mode, unused, realName;

			stream >> userName >> mode >> unused;
			std::getline(stream, realName);

			userName = Utils::trim(userName);
			mode = Utils::trim(mode);
			unused = Utils::trim(unused);
			realName = Utils::trim(realName);

			if (!realName.empty() && realName[0] == ':') {
				realName = realName.substr(1);
			}

			if (userName.empty() || userName.length() > USERLEN) {
				std::string response = ERR_NEEDMOREPARAMS;
				sendReplyOrError(client.getFd(), response);
				return ;
			}

			for (std::map<int, Client*>::iterator it = connections.begin(); it != connections.end(); it++) {
				if (it->second->getUser() == userName || !client.isCapNegotiation()) {
					std::string response = ERR_ALREADYREGISTERED;
					sendReplyOrError(client.getFd(), response);
					return ;
				}
			}

			client.setUser(userName);
			client.setUserRealName(realName);

			std::cout << "USER command received. Client username set to: " << userName << ", realname set to: " << realName << std::endl;
		}
		break;
	case QUIT:
		{
			std::istringstream stream(args);
			std::string reason, response;

			getline(stream, reason);
			reason = Utils::trim(reason);
			if (reason.empty()) {
				response = "Quit";
				std::cout << response << std::endl;
			} else {
				response = RPL_QUIT(reason);
				std::cout << response << std::endl;
			}
			// todo: notify other clients that share the same channel indicating that the client has exited the network
			
			sendReplyOrError(client.getFd(), ERROR(response));
			close(client.getFd());

			connections.erase(client.getFd());
			delete &client;

			std::cout << "QUIT command received. Client disconnected with the reason: " << response << std::endl;
		}
		break;
	}
}
