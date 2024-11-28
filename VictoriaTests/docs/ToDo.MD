# Vitoria's List
## Makefile
* create project tree
* add submodule locally 
* setup rules to work with branches
* create subfolder or brach make file 
* change git message to include current @git user
* Fix rules in gitTools/Makefile

## Server

* change nonblocking connection to check flags 0 and use return value from fcntl
* change from read() to recv()


# NOTES

### Using functor and std::for_each in channel

```cpp
class SendMessageFunctor
{
	private:
		Client* exclude;
		std::string message;

	public:
		SendMessageFunctor(Client* exclude, const std::string& message)
			: exclude(exclude), message(message) {}

		void operator()(Client* client) const
		{
			if (client != exclude)
			{
				client->sendMessage(message);
			}
		}
};
// std::for_each(members.begin(), members.end(), SendMessageFunctor(exclude, message));
```