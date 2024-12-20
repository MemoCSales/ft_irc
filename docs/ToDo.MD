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

## Client

- [x] Check python testHexChaty on server message
	[ ] Delete server info script and automatic message
- [ ] Check if client connection is a new file descriptor when joining a channel or making new connection, is 

## Channels

- [ ] Test one user join channel and kill server
- [ ] Test multiple users join channel and kill server
- [ ] User joint channel, another two did the same, send message to all, then first user was terminated and got this error
```bash
==================
WARNING: ThreadSanitizer: heap-use-after-free (pid=255453)
  Read of size 4 at 0x7b3000000000 by main thread:
    #0 Client::getSocket() const <null> (ircserv+0x4d1bc)
    #1 Channel::broadcast(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, Client*) <null> (ircserv+0x4c6ed)
    #2 Command::handlePrivMsg(Client&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, Channel*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, Channel*> > >&) <null> (ircserv+0x81c86)
    #3 Command::execute(Client&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, Channel*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, Channel*> > >&) <null> (ircserv+0x7c30c)
    #4 CommandParser::parseAndExecute(Client&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, Channel*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, Channel*> > >&) <null> (ircserv+0x7915f)
    #5 Client::handleRead() <null> (ircserv+0x51346)
    #6 Server::handleClient(int) <null> (ircserv+0x54d54)
    #7 Server::run() <null> (ircserv+0x556aa)
    #8 main <null> (ircserv+0x8d607)

  Previous write of size 8 at 0x7b3000000000 by main thread:
    [failed to restore the stack]

SUMMARY: ThreadSanitizer: heap-use-after-free (/home/lilizarr/42_Cursus/ft_irc/ircserv+0x4d1bc) in Client::getSocket() const
==================
```

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
- [ ] **401**: ERR_NOSUCHNICK - No such nick/channel.
- [ ] **403**: ERR_NOSUCHCHANNEL - No such channel.
- [ ] **404**: ERR_CANNOTSENDTOCHAN - Cannot send to channel.
- [ ] **421**: ERR_UNKNOWNCOMMAND - Unknown command.
- [ ] **433**: ERR_NICKNAMEINUSE - Nickname is already in use.

## EVAL

Basic Checks
- [x] Makefile and Compilation:

	Ensure the project compiles correctly using the Makefile.

	Verify the executable is named ircserv.

- [x] Single poll() Call:

	Confirm there is only one poll() call in the code, located in Server::run.

- [x] poll() Before accept, read/recv, write/send:

	Verify poll() is called before accept, recv, and send in the respective functions.

- [!] fcntl() Usage: ATENTION!!!!

	Ensure fcntl(fd, F_SETFL, O_NONBLOCK) is used correctly in Server::setNonBlocking.

### Networking Specials

- [x] Partial Commands:

	Use nc to send partial commands and verify the server handles them correctly.

- [x] Unexpected Client Kill:

	Kill a client unexpectedly and check if the server remains operational for other connections and new clients.

- [x] Half Command Sent:

	Use nc to send half a command and verify the server does not enter an odd state or get blocked.

- [ ] Stop Client and Flood Channel:  ATENTION!!

	Stop a client connected to a channel and flood the channel using another client. Ensure the server does not hang and processes stored commands when the client resumes.
	* ! Error happend when 3 users were in channel and operator disconected
		- [ ] Check if is solved when integrated main branch

### Client Commands Basic

- [x] Authentication, Nickname, Username, Join Channel:

	Test these functionalities using both nc and the reference IRC client.

- [x] Private Messages (PRIVMSG):

	Verify PRIVMSG functionality with different parameters.

### Client Commands Channel Operator

- [ ] Regular User Privileges:

	Ensure regular users do not have channel operator privileges.

- [ ] Channel Operator Commands:

	Test all channel operator commands using both nc and the reference IRC client.

### Memory Leaks

- [! ] Memory Leak Check:

	Check for memory leaks and data racings during various operations.

```bash
	==================
WARNING: ThreadSanitizer: data race (pid=208504)
  Read of size 4 at 0x7b0c00000020 by thread T1:
    #0 Server::sendPingToClients() <null> (ircserv+0x4d7b9)
    #1 pingTask(void*) <null> (ircserv+0x4db54)

  Previous write of size 8 at 0x7b0c00000020 by main thread:
    [failed to restore the stack]

  As if synchronized via sleep:
    #0 sleep ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:352 (libtsan.so.0+0x66757)
    #1 pingTask(void*) <null> (ircserv+0x4db23)

  Location is heap block of size 48 at 0x7b0c00000000 allocated by main thread:
    #0 operator new(unsigned long) ../../../../src/libsanitizer/tsan/tsan_new_delete.cpp:64 (libtsan.so.0+0x8f162)
    #1 __gnu_cxx::new_allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(unsigned long, void const*) <null> (ircserv+0x5b4ee)
    #2 __gnu_cxx::__alloc_traits<std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >, std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >&, unsigned long) <null> (ircserv+0x5b1f7)
    #3 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_get_node() <null> (ircserv+0x5a70d)
    #4 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_create_node(std::pair<int const, Client*> const&) <null> (ircserv+0x592ba)
    #5 std::_Rb_tree_node<std::pair<int const, Client*> >* std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node::operator()<std::pair<int const, Client*> >(std::pair<int const, Client*> const&) const <null> (ircserv+0x5878b)
    #6 std::_Rb_tree_iterator<std::pair<int const, Client*> > std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_<std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node>(std::_Rb_tree_node_base*, std::_Rb_tree_node_base*, std::pair<int const, Client*> const&, std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node&) <null> (ircserv+0x566e3)
    #7 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_unique(std::pair<int const, Client*> const&) <null> (ircserv+0x536de)
    #8 std::map<int, Client*, std::less<int>, std::allocator<std::pair<int const, Client*> > >::insert(std::pair<int const, Client*> const&) <null> (ircserv+0x50a60)
    #9 Server::handleNewConnection() <null> (ircserv+0x47ea3)
    #10 Server::run() <null> (ircserv+0x49182)
    #11 main <null> (ircserv+0x82748)

  Thread T1 (tid=208506, running) created by main thread at:
    #0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:969 (libtsan.so.0+0x605b8)
    #1 Server::startPingTask() <null> (ircserv+0x4dc18)
    #2 Server::Server(int&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) <null> (ircserv+0x46fbb)
    #3 main <null> (ircserv+0x82739)

SUMMARY: ThreadSanitizer: data race (/home/lilizarr/42_Cursus/ft_irc/ircserv+0x4d7b9) in Server::sendPingToClients()
==================
Sending PING to client: 4
==================
WARNING: ThreadSanitizer: data race (pid=208504)
  Read of size 8 at 0x7b0c00000028 by thread T1:
    #0 Server::sendPingToClients() <null> (ircserv+0x4d91d)
    #1 pingTask(void*) <null> (ircserv+0x4db54)

  Previous write of size 8 at 0x7b0c00000028 by main thread:
    [failed to restore the stack]

  As if synchronized via sleep:
    #0 sleep ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:352 (libtsan.so.0+0x66757)
    #1 pingTask(void*) <null> (ircserv+0x4db23)

  Location is heap block of size 48 at 0x7b0c00000000 allocated by main thread:
    #0 operator new(unsigned long) ../../../../src/libsanitizer/tsan/tsan_new_delete.cpp:64 (libtsan.so.0+0x8f162)
    #1 __gnu_cxx::new_allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(unsigned long, void const*) <null> (ircserv+0x5b4ee)
    #2 __gnu_cxx::__alloc_traits<std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >, std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >&, unsigned long) <null> (ircserv+0x5b1f7)
    #3 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_get_node() <null> (ircserv+0x5a70d)
    #4 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_create_node(std::pair<int const, Client*> const&) <null> (ircserv+0x592ba)
    #5 std::_Rb_tree_node<std::pair<int const, Client*> >* std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node::operator()<std::pair<int const, Client*> >(std::pair<int const, Client*> const&) const <null> (ircserv+0x5878b)
    #6 std::_Rb_tree_iterator<std::pair<int const, Client*> > std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_<std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node>(std::_Rb_tree_node_base*, std::_Rb_tree_node_base*, std::pair<int const, Client*> const&, std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node&) <null> (ircserv+0x566e3)
    #7 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_unique(std::pair<int const, Client*> const&) <null> (ircserv+0x536de)
    #8 std::map<int, Client*, std::less<int>, std::allocator<std::pair<int const, Client*> > >::insert(std::pair<int const, Client*> const&) <null> (ircserv+0x50a60)
    #9 Server::handleNewConnection() <null> (ircserv+0x47ea3)
    #10 Server::run() <null> (ircserv+0x49182)
    #11 main <null> (ircserv+0x82748)

  Thread T1 (tid=208506, running) created by main thread at:
    #0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:969 (libtsan.so.0+0x605b8)
    #1 Server::startPingTask() <null> (ircserv+0x4dc18)
    #2 Server::Server(int&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) <null> (ircserv+0x46fbb)
    #3 main <null> (ircserv+0x82739)

SUMMARY: ThreadSanitizer: data race (/home/lilizarr/42_Cursus/ft_irc/ircserv+0x4d91d) in Server::sendPingToClients()
==================
==================
WARNING: ThreadSanitizer: data race (pid=208504)
  Read of size 8 at 0x7ba000000040 by thread T1:
    #0 send ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:6640 (libtsan.so.0+0x54084)
    #1 Client::sendMessage(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) <null> (ircserv+0x4472b)
    #2 Server::sendPingToClients() <null> (ircserv+0x4d9a0)
    #3 pingTask(void*) <null> (ircserv+0x4db54)

  Previous write of size 8 at 0x7ba000000040 by main thread:
    [failed to restore the stack]

  As if synchronized via sleep:
    #0 sleep ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:352 (libtsan.so.0+0x66757)
    #1 pingTask(void*) <null> (ircserv+0x4db23)

  Location is file descriptor 4 created by main thread at:
    #0 accept ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:2862 (libtsan.so.0+0x5a794)
    #1 Server::handleNewConnection() <null> (ircserv+0x47ac0)
    #2 Server::run() <null> (ircserv+0x49182)
    #3 main <null> (ircserv+0x82748)

  Thread T1 (tid=208506, running) created by main thread at:
    #0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:969 (libtsan.so.0+0x605b8)
    #1 Server::startPingTask() <null> (ircserv+0x4dc18)
    #2 Server::Server(int&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) <null> (ircserv+0x46fbb)
    #3 main <null> (ircserv+0x82739)

SUMMARY: ThreadSanitizer: data race (/home/lilizarr/42_Cursus/ft_irc/ircserv+0x4472b) in Client::sendMessage(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
==================
==================
WARNING: ThreadSanitizer: data race (pid=208504)
  Read of size 4 at 0x7b0c00000080 by thread T1:
    #0 Server::sendPingToClients() <null> (ircserv+0x4d7b9)
    #1 pingTask(void*) <null> (ircserv+0x4db54)

  Previous write of size 8 at 0x7b0c00000080 by main thread:
    #0 operator new(unsigned long) ../../../../src/libsanitizer/tsan/tsan_new_delete.cpp:64 (libtsan.so.0+0x8f162)
    #1 __gnu_cxx::new_allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(unsigned long, void const*) <null> (ircserv+0x5b4ee)
    #2 __gnu_cxx::__alloc_traits<std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >, std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >&, unsigned long) <null> (ircserv+0x5b1f7)
    #3 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_get_node() <null> (ircserv+0x5a70d)
    #4 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_create_node(std::pair<int const, Client*> const&) <null> (ircserv+0x592ba)
    #5 std::_Rb_tree_node<std::pair<int const, Client*> >* std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node::operator()<std::pair<int const, Client*> >(std::pair<int const, Client*> const&) const <null> (ircserv+0x5878b)
    #6 std::_Rb_tree_iterator<std::pair<int const, Client*> > std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_<std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node>(std::_Rb_tree_node_base*, std::_Rb_tree_node_base*, std::pair<int const, Client*> const&, std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node&) <null> (ircserv+0x566e3)
    #7 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_unique(std::pair<int const, Client*> const&) <null> (ircserv+0x536de)
    #8 std::map<int, Client*, std::less<int>, std::allocator<std::pair<int const, Client*> > >::insert(std::pair<int const, Client*> const&) <null> (ircserv+0x50a60)
    #9 Server::handleNewConnection() <null> (ircserv+0x47ea3)
    #10 Server::run() <null> (ircserv+0x49182)
    #11 main <null> (ircserv+0x82748)

  As if synchronized via sleep:
    #0 sleep ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:352 (libtsan.so.0+0x66757)
    #1 pingTask(void*) <null> (ircserv+0x4db23)

  Location is heap block of size 48 at 0x7b0c00000060 allocated by main thread:
    #0 operator new(unsigned long) ../../../../src/libsanitizer/tsan/tsan_new_delete.cpp:64 (libtsan.so.0+0x8f162)
    #1 __gnu_cxx::new_allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(unsigned long, void const*) <null> (ircserv+0x5b4ee)
    #2 __gnu_cxx::__alloc_traits<std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >, std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >&, unsigned long) <null> (ircserv+0x5b1f7)
    #3 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_get_node() <null> (ircserv+0x5a70d)
    #4 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_create_node(std::pair<int const, Client*> const&) <null> (ircserv+0x592ba)
    #5 std::_Rb_tree_node<std::pair<int const, Client*> >* std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node::operator()<std::pair<int const, Client*> >(std::pair<int const, Client*> const&) const <null> (ircserv+0x5878b)
    #6 std::_Rb_tree_iterator<std::pair<int const, Client*> > std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_<std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node>(std::_Rb_tree_node_base*, std::_Rb_tree_node_base*, std::pair<int const, Client*> const&, std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node&) <null> (ircserv+0x566e3)
    #7 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_unique(std::pair<int const, Client*> const&) <null> (ircserv+0x536de)
    #8 std::map<int, Client*, std::less<int>, std::allocator<std::pair<int const, Client*> > >::insert(std::pair<int const, Client*> const&) <null> (ircserv+0x50a60)
    #9 Server::handleNewConnection() <null> (ircserv+0x47ea3)
    #10 Server::run() <null> (ircserv+0x49182)
    #11 main <null> (ircserv+0x82748)

  Thread T1 (tid=208506, running) created by main thread at:
    #0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:969 (libtsan.so.0+0x605b8)
    #1 Server::startPingTask() <null> (ircserv+0x4dc18)
    #2 Server::Server(int&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) <null> (ircserv+0x46fbb)
    #3 main <null> (ircserv+0x82739)

SUMMARY: ThreadSanitizer: data race (/home/lilizarr/42_Cursus/ft_irc/ircserv+0x4d7b9) in Server::sendPingToClients()
==================
Sending PING to client: 5
==================
WARNING: ThreadSanitizer: data race (pid=208504)
  Read of size 8 at 0x7b0c00000088 by thread T1:
    #0 Server::sendPingToClients() <null> (ircserv+0x4d91d)
    #1 pingTask(void*) <null> (ircserv+0x4db54)

  Previous write of size 8 at 0x7b0c00000088 by main thread:
    #0 operator new(unsigned long) ../../../../src/libsanitizer/tsan/tsan_new_delete.cpp:64 (libtsan.so.0+0x8f162)
    #1 __gnu_cxx::new_allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(unsigned long, void const*) <null> (ircserv+0x5b4ee)
    #2 __gnu_cxx::__alloc_traits<std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >, std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >&, unsigned long) <null> (ircserv+0x5b1f7)
    #3 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_get_node() <null> (ircserv+0x5a70d)
    #4 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_create_node(std::pair<int const, Client*> const&) <null> (ircserv+0x592ba)
    #5 std::_Rb_tree_node<std::pair<int const, Client*> >* std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node::operator()<std::pair<int const, Client*> >(std::pair<int const, Client*> const&) const <null> (ircserv+0x5878b)
    #6 std::_Rb_tree_iterator<std::pair<int const, Client*> > std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_<std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node>(std::_Rb_tree_node_base*, std::_Rb_tree_node_base*, std::pair<int const, Client*> const&, std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node&) <null> (ircserv+0x566e3)
    #7 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_unique(std::pair<int const, Client*> const&) <null> (ircserv+0x536de)
    #8 std::map<int, Client*, std::less<int>, std::allocator<std::pair<int const, Client*> > >::insert(std::pair<int const, Client*> const&) <null> (ircserv+0x50a60)
    #9 Server::handleNewConnection() <null> (ircserv+0x47ea3)
    #10 Server::run() <null> (ircserv+0x49182)
    #11 main <null> (ircserv+0x82748)

  As if synchronized via sleep:
    #0 sleep ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:352 (libtsan.so.0+0x66757)
    #1 pingTask(void*) <null> (ircserv+0x4db23)

  Location is heap block of size 48 at 0x7b0c00000060 allocated by main thread:
    #0 operator new(unsigned long) ../../../../src/libsanitizer/tsan/tsan_new_delete.cpp:64 (libtsan.so.0+0x8f162)
    #1 __gnu_cxx::new_allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(unsigned long, void const*) <null> (ircserv+0x5b4ee)
    #2 __gnu_cxx::__alloc_traits<std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >, std::_Rb_tree_node<std::pair<int const, Client*> > >::allocate(std::allocator<std::_Rb_tree_node<std::pair<int const, Client*> > >&, unsigned long) <null> (ircserv+0x5b1f7)
    #3 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_get_node() <null> (ircserv+0x5a70d)
    #4 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_create_node(std::pair<int const, Client*> const&) <null> (ircserv+0x592ba)
    #5 std::_Rb_tree_node<std::pair<int const, Client*> >* std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node::operator()<std::pair<int const, Client*> >(std::pair<int const, Client*> const&) const <null> (ircserv+0x5878b)
    #6 std::_Rb_tree_iterator<std::pair<int const, Client*> > std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_<std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node>(std::_Rb_tree_node_base*, std::_Rb_tree_node_base*, std::pair<int const, Client*> const&, std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_Alloc_node&) <null> (ircserv+0x566e3)
    #7 std::_Rb_tree<int, std::pair<int const, Client*>, std::_Select1st<std::pair<int const, Client*> >, std::less<int>, std::allocator<std::pair<int const, Client*> > >::_M_insert_unique(std::pair<int const, Client*> const&) <null> (ircserv+0x536de)
    #8 std::map<int, Client*, std::less<int>, std::allocator<std::pair<int const, Client*> > >::insert(std::pair<int const, Client*> const&) <null> (ircserv+0x50a60)
    #9 Server::handleNewConnection() <null> (ircserv+0x47ea3)
    #10 Server::run() <null> (ircserv+0x49182)
    #11 main <null> (ircserv+0x82748)

  Thread T1 (tid=208506, running) created by main thread at:
    #0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:969 (libtsan.so.0+0x605b8)
    #1 Server::startPingTask() <null> (ircserv+0x4dc18)
    #2 Server::Server(int&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) <null> (ircserv+0x46fbb)
    #3 main <null> (ircserv+0x82739)

SUMMARY: ThreadSanitizer: data race (/home/lilizarr/42_Cursus/ft_irc/ircserv+0x4d91d) in Server::sendPingToClients()
==================
==================
WARNING: ThreadSanitizer: data race (pid=208504)
  Read of size 8 at 0x7ba000000050 by thread T1:
    #0 send ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:6640 (libtsan.so.0+0x54084)
    #1 Client::sendMessage(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) <null> (ircserv+0x4472b)
    #2 Server::sendPingToClients() <null> (ircserv+0x4d9a0)
    #3 pingTask(void*) <null> (ircserv+0x4db54)

  Previous write of size 8 at 0x7ba000000050 by main thread:
    #0 accept ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:2862 (libtsan.so.0+0x5a794)
    #1 Server::handleNewConnection() <null> (ircserv+0x47ac0)
    #2 Server::run() <null> (ircserv+0x49182)
    #3 main <null> (ircserv+0x82748)

  As if synchronized via sleep:
    #0 sleep ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:352 (libtsan.so.0+0x66757)
    #1 pingTask(void*) <null> (ircserv+0x4db23)

  Location is file descriptor 5 created by main thread at:
    #0 accept ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:2862 (libtsan.so.0+0x5a794)
    #1 Server::handleNewConnection() <null> (ircserv+0x47ac0)
    #2 Server::run() <null> (ircserv+0x49182)
    #3 main <null> (ircserv+0x82748)

  Thread T1 (tid=208506, running) created by main thread at:
    #0 pthread_create ../../../../src/libsanitizer/tsan/tsan_interceptors_posix.cpp:969 (libtsan.so.0+0x605b8)
    #1 Server::startPingTask() <null> (ircserv+0x4dc18)
    #2 Server::Server(int&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) <null> (ircserv+0x46fbb)
    #3 main <null> (ircserv+0x82739)

SUMMARY: ThreadSanitizer: data race (/home/lilizarr/42_Cursus/ft_irc/ircserv+0x4472b) in Client::sendMessage(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
==================
Sending PING to client: 4
Sending PING to client: 5
Received message: SJO

```

### Additional Tests

- [ ] File Transfer (if implemented):

	Test file transfer functionality with the reference IRC client.

- [ ] IRC Bot (if implemented):

	Test the IRC bot functionality.
