#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <poll.h>
#include <ctime>

#include "Client.hpp"
#include "Channel.hpp"

class Server
{
public:
	typedef void (Server::*cmdHandler)(Client &client, const std::vector<std::string> &args);

	Server(int port, const std::string &password);
	~Server();

	/* --- lifecycle --- */
	void	run();
	void	stop();

	/* --- getters --- */
	const std::string	&getPassword() const;
	const std::string	&getServerName() const;
	std::string			getCreationTime() const;
	Client				*getClientByNick(const std::string &nick);
	Channel				*getChannel(const std::string &name);
	Channel				*createChannel(const std::string &name, Client *creator);
	void				removeChannel(Channel *ch);
	const std::set<std::string>	&getChannelNames() const;

	/* --- messaging --- */
	void	sendTo(Client &client, const std::string &message);
	void	broadcastToChannel(Channel &channel, const std::string &message, Client *exclude);

	/* --- command handling --- */
	void	handleCommand(Client &client, const std::string &rawLine);

private:
	Server();
	Server(const Server &);
	Server &operator=(const Server &);

	/* --- network setup --- */
	void	initSocket();
	void	acceptClient();
	void	handleRead(Client &client);
	void	handleWrite(Client &client);
	void	disconnectClient(Client &client, const std::string &reason);
	void	removeClient(Client &client);

	/* --- command dispatch --- */
	void	dispatch(Client &client, const std::string &cmd, const std::vector<std::string> &args);
	void	registerHandlers();

	/* --- registration state machine --- */
	void	tryRegister(Client &client);

	/* --- command handlers --- */
	void	cmdPass(Client &client, const std::vector<std::string> &args);
	void	cmdNick(Client &client, const std::vector<std::string> &args);
	void	cmdUser(Client &client, const std::vector<std::string> &args);
	void	cmdQuit(Client &client, const std::vector<std::string> &args);
	void	cmdCap(Client &client, const std::vector<std::string> &args);
	void	cmdPing(Client &client, const std::vector<std::string> &args);
	void	cmdPong(Client &client, const std::vector<std::string> &args);

	void	cmdJoin(Client &client, const std::vector<std::string> &args);
	void	cmdPart(Client &client, const std::vector<std::string> &args);
	void	cmdPrivMsg(Client &client, const std::vector<std::string> &args);
	void	cmdNotice(Client &client, const std::vector<std::string> &args);
	void	cmdKick(Client &client, const std::vector<std::string> &args);
	void	cmdInvite(Client &client, const std::vector<std::string> &args);
	void	cmdTopic(Client &client, const std::vector<std::string> &args);
	void	cmdMode(Client &client, const std::vector<std::string> &args);

	/* --- members --- */
	int							_port;
	std::string					_password;
	std::string					_serverName;
	std::time_t					_creationTime;

	int							_listenFd;
	static bool					_signalCaught;
	static void					handleSignal(int);

	std::vector<pollfd>			_pollfds;
	std::map<int, Client *>		_clients;		// fd -> client
	std::map<std::string, Channel *> _channels;	// name -> channel

	std::map<std::string, cmdHandler>	_handlers;
};

/* small helpers */
bool	isValidChannelName(const std::string &name);
bool	isValidNick(const std::string &nick);
bool	isAllDigits(const std::string &s);

#endif
