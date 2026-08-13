#include "Server.hpp"
#include "Reply.hpp"

#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <csignal>

bool Server::_signalCaught = false;

void	Server::handleSignal(int)
{
	_signalCaught = true;
}

/* ------------------------------------------------------------------ */
/*                      small free helpers                             */
/* ------------------------------------------------------------------ */

bool	isAllDigits(const std::string &s)
{
	if (s.empty())
		return false;
	for (size_t i = 0; i < s.size(); ++i)
		if (!std::isdigit(static_cast<unsigned char>(s[i])))
			return false;
	return true;
}

bool	isValidChannelName(const std::string &name)
{
	if (name.size() < 2 || name[0] != '#')
		return false;
	for (size_t i = 1; i < name.size(); ++i)
	{
		char c = name[i];
		if (c == ' ' || c == ',' || c == '\a' || c == 0x07 || c == '\0' || c == ':')
			return false;
	}
	return true;
}

bool	isValidNick(const std::string &nick)
{
	static const std::string special = "[]\\`_^{|}";

	if (nick.empty() || nick.size() > 9)
		return false;
	if (!std::isalpha(static_cast<unsigned char>(nick[0])) &&
		special.find(nick[0]) == std::string::npos)
		return false;
	for (size_t i = 1; i < nick.size(); ++i)
	{
		char c = nick[i];
		if (!std::isalnum(static_cast<unsigned char>(c)) &&
			c != '-' && special.find(c) == std::string::npos)
			return false;
	}
	return true;
}

/*
 * Split an IRC line into tokens:
 *   "CMD p1 p2 :trailing text" -> { "CMD", "p1", "p2", "trailing text" }
 * The trailing parameter (after " :") is kept as one token, spaces included.
 */
static std::vector<std::string>	splitLine(const std::string &line)
{
	std::vector<std::string> tokens;
	size_t start = 0;

	while (start < line.size())
	{
		if (line[start] == ':')
		{
			tokens.push_back(line.substr(start + 1));
			break;
		}
		size_t end = line.find(' ', start);
		if (end == std::string::npos)
		{
			tokens.push_back(line.substr(start));
			break;
		}
		if (end > start)
			tokens.push_back(line.substr(start, end - start));
		start = line.find_first_not_of(' ', end);
		if (start == std::string::npos)
			break;
	}
	return tokens;
}

/* ------------------------------------------------------------------ */
/*                      construction / destruction                     */
/* ------------------------------------------------------------------ */

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _serverName("irc.42.fr"),
	  _creationTime(std::time(NULL)), _listenFd(-1)
{
	initSocket();
	registerHandlers();
}

Server::~Server()
{
	std::map<int, Client *>::iterator it = _clients.begin();
	for (; it != _clients.end(); ++it)
		delete it->second;
	std::map<std::string, Channel *>::iterator ch = _channels.begin();
	for (; ch != _channels.end(); ++ch)
		delete ch->second;
	if (_listenFd != -1)
		close(_listenFd);
}

/* ------------------------------------------------------------------ */
/*                      socket setup                                   */
/* ------------------------------------------------------------------ */

void	Server::initSocket()
{
	int opt = 1;

	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd < 0)
	{
		std::cerr << "Error: socket() failed: " << std::strerror(errno) << std::endl;
		std::exit(1);
	}
	if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Error: setsockopt() failed: " << std::strerror(errno) << std::endl;
		std::exit(1);
	}
	if (fcntl(_listenFd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error: fcntl() failed: " << std::strerror(errno) << std::endl;
		std::exit(1);
	}

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(_port);

	if (bind(_listenFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
	{
		std::cerr << "Error: bind() failed: " << std::strerror(errno) << std::endl;
		std::exit(1);
	}
	if (listen(_listenFd, SOMAXCONN) < 0)
	{
		std::cerr << "Error: listen() failed: " << std::strerror(errno) << std::endl;
		std::exit(1);
	}
	std::cout << "ircserv listening on port " << _port << std::endl;
}

/* ------------------------------------------------------------------ */
/*                      main poll loop                                 */
/* ------------------------------------------------------------------ */

void	Server::run()
{
	_signalCaught = false;
	::signal(SIGPIPE, SIG_IGN);
	::signal(SIGINT, Server::handleSignal);

	while (!_signalCaught)
	{
		_pollfds.clear();

		pollfd listenPoll;
		listenPoll.fd = _listenFd;
		listenPoll.events = POLLIN;
		listenPoll.revents = 0;
		_pollfds.push_back(listenPoll);

		std::map<int, Client *>::iterator it;
		for (it = _clients.begin(); it != _clients.end(); ++it)
		{
			pollfd p;
			p.fd = it->first;
			p.events = POLLIN;
			p.revents = 0;
			if (it->second->hasPendingOutput())
				p.events |= POLLOUT;
			_pollfds.push_back(p);
		}

		if (poll(&_pollfds[0], _pollfds.size(), -1) < 0)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "Error: poll() failed: " << std::strerror(errno) << std::endl;
			break;
		}

		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].revents == 0)
				continue;
			if (_pollfds[i].fd == _listenFd)
			{
				if (_pollfds[i].revents & POLLIN)
					acceptClient();
				continue;
			}
			Client *client = _clients[_pollfds[i].fd];
			if (!client)
				continue;
			if (_pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				disconnectClient(*client, "");
			else
			{
				if (_pollfds[i].revents & POLLIN)
					handleRead(*client);
				if (_pollfds[i].revents & POLLOUT)
				{
					/* the client may have been deleted by handleRead */
					std::map<int, Client *>::iterator still = _clients.find(_pollfds[i].fd);
					if (still != _clients.end())
						handleWrite(*still->second);
				}
			}
		}
	}

	/* graceful shutdown on SIGINT */
	std::map<int, Client *>::iterator c = _clients.begin();
	for (; c != _clients.end(); )
	{
		Client *client = c->second;
		++c;
		disconnectClient(*client, "Server shutting down");
	}
}

void	Server::stop() { _signalCaught = true; }

/* ------------------------------------------------------------------ */
/*                      connection handling                            */
/* ------------------------------------------------------------------ */

void	Server::acceptClient()
{
	sockaddr_in addr;
	socklen_t len = sizeof(addr);
	int fd;

	while (true)
	{
		fd = accept(_listenFd, reinterpret_cast<sockaddr *>(&addr), &len);
		if (fd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
			std::cerr << "Error: accept() failed: " << std::strerror(errno) << std::endl;
			return;
		}
		if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		{
			close(fd);
			return;
		}
		std::string ip = inet_ntoa(addr.sin_addr);
		Client *client = new Client(fd, ip);
		_clients[fd] = client;
		std::cout << "New connection from " << ip << " (fd " << fd << ")" << std::endl;
	}
}

void	Server::handleRead(Client &client)
{
	char buf[1024];

	int ret = recv(client.getFd(), buf, sizeof(buf), 0);
	if (ret <= 0)
	{
		disconnectClient(client, "Connection closed");
		return;
	}
	client.appendReadBuffer(std::string(buf, ret));

	int fd = client.getFd();
	std::string line;
	while (client.extractCommand(line))
	{
		if (!line.empty())
		{
			handleCommand(client, line);
			/* a command (PASS error, QUIT) may have deleted the client */
			if (_clients.find(fd) == _clients.end())
				return;
		}
	}
}

void	Server::handleWrite(Client &client)
{
	std::string out = client.takeOutput();

	if (out.empty())
		return;
	int ret = send(client.getFd(), out.c_str(), out.size(), 0);
	if (ret < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			client.queueOutput(out); // try again on next POLLOUT
			return;
		}
		disconnectClient(client, "");
		return;
	}
	if (static_cast<size_t>(ret) < out.size())
		client.queueOutput(out.substr(ret)); // partial write: requeue the rest
}

void	Server::disconnectClient(Client &client, const std::string &reason)
{
	std::string quitMsg = ":" + client.getPrefix() + " QUIT";
	if (!reason.empty())
		quitMsg += " :" + reason;
	quitMsg += "\r\n";

	std::set<Channel *>::iterator it = client.getChannels().begin();
	for (; it != client.getChannels().end(); )
	{
		Channel *ch = *it;
		++it;
		ch->broadcast(*this, quitMsg, &client);
		ch->removeMember(&client);
		if (ch->memberCount() == 0)
		{
			_channels.erase(ch->getName());
			delete ch;
		}
	}
	removeClient(client);
}

void	Server::removeClient(Client &client)
{
	int fd = client.getFd();
	/* best effort: flush anything still queued (e.g. ERR_PASSWDMISMATCH) */
	std::string pending = client.takeOutput();
	if (!pending.empty())
		::send(fd, pending.c_str(), pending.size(), 0);
	_clients.erase(fd);
	close(fd);
	delete &client;
}

/* ------------------------------------------------------------------ */
/*                      messaging helpers                              */
/* ------------------------------------------------------------------ */

void	Server::sendTo(Client &client, const std::string &message)
{
	client.queueOutput(message);
}

void	Server::broadcastToChannel(Channel &channel, const std::string &message, Client *exclude)
{
	channel.broadcast(*this, message, exclude);
}

/* ------------------------------------------------------------------ */
/*                      lookup helpers                                 */
/* ------------------------------------------------------------------ */

Client	*Server::getClientByNick(const std::string &nick)
{
	std::map<int, Client *>::iterator it;
	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNick() == nick)
			return it->second;
	}
	return 0;
}

Channel	*Server::getChannel(const std::string &name)
{
	std::map<std::string, Channel *>::iterator it = _channels.find(name);
	if (it == _channels.end())
		return 0;
	return it->second;
}

Channel	*Server::createChannel(const std::string &name, Client *creator)
{
	Channel *ch = new Channel(name, creator);
	_channels[name] = ch;
	return ch;
}

void	Server::removeChannel(Channel *ch)
{
	if (!ch)
		return;
	_channels.erase(ch->getName());
	delete ch;
}

const std::set<std::string>	&Server::getChannelNames() const
{
	static std::set<std::string> names;
	names.clear();
	std::map<std::string, Channel *>::const_iterator it;
	for (it = _channels.begin(); it != _channels.end(); ++it)
		names.insert(it->first);
	return names;
}

const std::string	&Server::getPassword() const { return _password; }
const std::string	&Server::getServerName() const { return _serverName; }

std::string	Server::getCreationTime() const
{
	return std::string(std::ctime(&_creationTime));
}

/* ------------------------------------------------------------------ */
/*                      command parsing / dispatch                     */
/* ------------------------------------------------------------------ */

void	Server::handleCommand(Client &client, const std::string &rawLine)
{
	std::vector<std::string> tokens = splitLine(rawLine);
	if (tokens.empty())
		return;

	std::string cmd = tokens[0];
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

	std::vector<std::string> args(tokens.begin() + 1, tokens.end());
	dispatch(client, cmd, args);
}

void	Server::dispatch(Client &client, const std::string &cmd,
					   const std::vector<std::string> &args)
{
	std::map<std::string, cmdHandler>::iterator it = _handlers.find(cmd);
	if (it == _handlers.end())
	{
		sendTo(client, Reply::errUnknownCommand(_serverName, client.getNick(), cmd));
		return;
	}
	(this->*(it->second))(client, args);
}

void	Server::registerHandlers()
{
	_handlers["PASS"] = &Server::cmdPass;
	_handlers["NICK"] = &Server::cmdNick;
	_handlers["USER"] = &Server::cmdUser;
	_handlers["QUIT"] = &Server::cmdQuit;
	_handlers["CAP"] = &Server::cmdCap;
	_handlers["PING"] = &Server::cmdPing;
	_handlers["PONG"] = &Server::cmdPong;
	_handlers["JOIN"] = &Server::cmdJoin;
	_handlers["PART"] = &Server::cmdPart;
	_handlers["PRIVMSG"] = &Server::cmdPrivMsg;
	_handlers["NOTICE"] = &Server::cmdNotice;
	_handlers["KICK"] = &Server::cmdKick;
	_handlers["INVITE"] = &Server::cmdInvite;
	_handlers["TOPIC"] = &Server::cmdTopic;
	_handlers["MODE"] = &Server::cmdMode;
}

/* ------------------------------------------------------------------ */
/*                      registration                                   */
/* ------------------------------------------------------------------ */

void	Server::tryRegister(Client &client)
{
	if (client.isWelcomed() || !client.registrationComplete())
		return;

	client.setWelcomed(true);
	sendTo(client, Reply::rplWelcome(_serverName, client.getNick(),
									 client.getUser(), client.getHostname()));
	sendTo(client, Reply::numbered(RPL_YOURHOST, _serverName, client.getNick(),
								   ":Your host is " + _serverName + ", running version ircserv-1.0"));
	sendTo(client, Reply::numbered(RPL_CREATED, _serverName, client.getNick(),
								   ":This server was created " + getCreationTime()));
	sendTo(client, Reply::numbered(RPL_MYINFO, _serverName, client.getNick(),
								   _serverName + " ircserv-1.0 o itkol"));
}

/* ------------------------------------------------------------------ */
/*                      basic commands                                 */
/* ------------------------------------------------------------------ */

void	Server::cmdCap(Client &client, const std::vector<std::string> &args)
{
	if (args.empty())
		return;
	if (args[0] == "LS")
		sendTo(client, std::string("CAP * LS :\r\n"));
	else if (args[0] == "REQ" && args.size() > 1)
		sendTo(client, "CAP * NAK :" + args[1] + "\r\n");
	/* "END" and others: nothing to do */
}

void	Server::cmdPass(Client &client, const std::vector<std::string> &args)
{
	if (client.registrationComplete())
	{
		sendTo(client, Reply::errAlreadyRegistered(_serverName, client.getNick()));
		return;
	}
	if (args.empty())
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "PASS"));
		return;
	}
	if (args[0] != _password)
	{
		sendTo(client, Reply::errPasswordMismatch(_serverName));
		disconnectClient(client, "Wrong password");
		return;
	}
	client.setPassOk(true);
	tryRegister(client);
}

void	Server::cmdNick(Client &client, const std::vector<std::string> &args)
{
	if (args.empty() || args[0].empty())
	{
		sendTo(client, Reply::errNoNicknameGiven(_serverName));
		return;
	}
	const std::string &newNick = args[0];
	if (!isValidNick(newNick))
	{
		sendTo(client, Reply::errErroneusNickname(_serverName, newNick));
		return;
	}
	Client *existing = getClientByNick(newNick);
	if (existing && existing != &client)
	{
		sendTo(client, Reply::errNicknameInUse(_serverName, newNick));
		return;
	}

	std::string oldPrefix = client.nickSet() ? client.getPrefix() : "";

	client.setNick(newNick);
	if (!client.isWelcomed())
	{
		tryRegister(client);
		return;
	}
	/* nick change after registration: notify shared channels */
	std::string msg = ":" + oldPrefix + " NICK :" + newNick + "\r\n";
	std::set<Channel *>::iterator it = client.getChannels().begin();
	for (; it != client.getChannels().end(); ++it)
		(*it)->broadcast(*this, msg);
	sendTo(client, msg);
}

void	Server::cmdUser(Client &client, const std::vector<std::string> &args)
{
	if (client.registrationComplete())
	{
		sendTo(client, Reply::errAlreadyRegistered(_serverName, client.getNick()));
		return;
	}
	if (args.size() < 4)
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "USER"));
		return;
	}
	client.setUser(args[0]);
	client.setRealName(args[3]);
	tryRegister(client);
}

void	Server::cmdQuit(Client &client, const std::vector<std::string> &args)
{
	std::string reason;
	if (!args.empty())
		reason = args[0];
	disconnectClient(client, reason);
}

void	Server::cmdPing(Client &client, const std::vector<std::string> &args)
{
	std::string token = args.empty() ? "" : args[0];
	sendTo(client, "PONG :" + token + "\r\n");
}

void	Server::cmdPong(Client &client, const std::vector<std::string> &args)
{
	(void)client;
	(void)args;
}
