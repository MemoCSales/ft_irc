# Vitoria's List

## Makefile

- [x] setup rules to work with branches
- [x] change git message to include current @git user

## DOCS
- [x] Fix rules in gitTools/Makefile


## Server

- [x] change nonblocking connection to check flags 0 and use return value from fcntl
- [x] change from read() to recv()
- [x] Check if ip and port are restricted values or allowed, set up warnings?
- [ ] If connection is lost with all clients, clean screen?
- [ ] when debug=true, keep log with connections
- [ ] check with helgrind for threads and file descriptors

```bash
	For setting up a local network, such as an IRC server, you can use any available port that is not already in use by another service. Commonly used ports for IRC servers are:

- **6667**: The default port for IRC.
- **6660-6669**: A range of ports often used for IRC.
- **7000-7005**: Another range of ports sometimes used for IRC.
```
## Client
- [x] Check python testHexChaty on server message
	[ ] Delete server info script and automatic message
- [ ] Check if client connection is a new file descriptor when joining a channel or making new connection, is 
# NOTES

In IRC, server replies are identified by numeric codes. These codes are used to convey various types of information, such as connection status, errors, and responses to commands. Here are some common numeric reply codes that you might find useful when hooking server events in HexChat:

### Connection and Registration Replies
- [x] **001**: RPL_WELCOME - Welcome message after successful login.
- [ ] **002**: RPL_YOURHOST - Information about the server.
- [ ] **003**: RPL_CREATED - Server creation date.
- [ ] **004**: RPL_MYINFO - Server information.
- [ ] **005**: RPL_ISUPPORT - Supported features.

### Channel Replies
- [ ] **324**: RPL_CHANNELMODEIS - Channel modes.
- [ ] **332**: RPL_TOPIC - Channel topic.
- [ ] **333**: RPL_TOPICWHOTIME - Topic setter and time.
- [ ] **353**: RPL_NAMREPLY - List of users in a channel.
- [ ] **366**: RPL_ENDOFNAMES - End of NAMES list.

### User Replies
- [ ] **301**: RPL_AWAY - User is marked as away.
- [ ] **311**: RPL_WHOISUSER - WHOIS user information.
- [ ] **312**: RPL_WHOISSERVER - WHOIS server information.
- [ ] **313**: RPL_WHOISOPERATOR - WHOIS operator information.
- [ ] **317**: RPL_WHOISIDLE - WHOIS idle time.
- [ ] **318**: RPL_ENDOFWHOIS - End of WHOIS list.
- [ ] **319**: RPL_WHOISCHANNELS - Channels the user is in.

### Error Replies
- **401**: ERR_NOSUCHNICK - No such nick/channel.
- **403**: ERR_NOSUCHCHANNEL - No such channel.
- **404**: ERR_CANNOTSENDTOCHAN - Cannot send to channel.
- **421**: ERR_UNKNOWNCOMMAND - Unknown command.
- **433**: ERR_NICKNAMEINUSE - Nickname is already in use.
