#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>

class Channel;

class Client
{
public:
	Client(int fd, const std::string &ip);
	~Client();

	/* --- getters --- */
	int							getFd() const;
	const std::string			&getIp() const;
	const std::string			&getNick() const;
	const std::string			&getUser() const;
	const std::string			&getRealName() const;
	const std::string			&getHostname() const;
	const std::string			&getPrefix() const; // nick!user@host
	bool						isRegistered() const;
	bool						isWelcomed() const;
	const std::set<Channel *>	&getChannels() const;
	const std::set<std::string>	&getInvitedChannels() const;
	bool						hasPendingOutput() const;

	/* --- setters / state --- */
	void	setNick(const std::string &nick);
	void	setUser(const std::string &user);
	void	setRealName(const std::string &real);
	void	setPassOk(bool ok);
	void	setWelcomed(bool welcomed);
	bool	passOk() const;
	bool	nickSet() const;
	bool	userSet() const;
	bool	registrationComplete() const; // PASS + NICK + USER received
	void	markConnected();

	/* --- channel membership --- */
	void	joinChannel(Channel *ch);
	void	leaveChannel(Channel *ch);
	void	addInvite(const std::string &channelName);
	void	removeInvite(const std::string &channelName);
	bool	isInvitedTo(const std::string &channelName) const;

	/* --- I/O buffers (aggregation / write queue) --- */
	void	appendReadBuffer(const std::string &data);
	bool	extractCommand(std::string &cmd); // fills cmd with one full IRC line if available
	void	clearReadBuffer();

	void	queueOutput(const std::string &msg); // append to write queue
	std::string	takeOutput(); // returns and clears the whole write queue

	/* --- misc --- */
	bool	operator==(const Client &other) const;

private:
	Client();
	Client(const Client &);
	Client &operator=(const Client &);

	int						_fd;
	std::string				_ip;
	std::string				_nick;
	std::string				_user;
	std::string				_realname;
	std::string				_hostname;

	bool					_passOk;
	bool					_nickSet;
	bool					_userSet;
	bool					_welcomed;

	std::string				_readBuffer;  // aggregates partial data until \r\n
	std::string				_writeBuffer; // output queue

	std::set<Channel *>		_channels;
	std::set<std::string>	_invited;     // channel names this client was invited to
};

#endif
