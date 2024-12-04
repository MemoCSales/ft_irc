# Vitoria's List
## Makefile
- [ ] create project tree
- [x] setup rules to work with branches
- [x] change git message to include current @git user

## DOCS
- [x] Fix rules in gitTools/Makefile

## TICKETS
- [ ] Set ticket request with problems found?

## Server

- [x] change nonblocking connection to check flags 0 and use return value from fcntl
- [x] change from read() to recv()
- [x] Check if ip and port are restricted values or allowed, set up warnings?
- [ ] If connection is lost with all clients, clean screen
- [ ] when debug=true, keep log with connections
- [ ] check with helgrind for threads and file descriptors

## Client
- [ ] Check if client connection is a new file descriptor when joining a channel or making new connection, is 
# NOTES