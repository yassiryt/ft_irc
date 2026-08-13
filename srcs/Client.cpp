#include "Client.hpp"

#include <sys/socket.h>

Client::Client(int fd, const std::string &ip)
	: _fd(fd), _ip(ip), _hostname(ip),
	  _passOk(false), _nickSet(false), _userSet(false), _welcomed(false)
{
}

Client::~Client()
{
}

/* ---------------- getters ---------------- */

int	Client::getFd() const { return _fd; }
const std::string &Client::getIp() const { return _ip; }
const std::string &Client::getNick() const { return _nick; }
const std::string &Client::getUser() const { return _user; }
const std::string &Client::getRealName() const { return _realname; }
const std::string &Client::getHostname() const { return _hostname; }

const std::string &Client::getPrefix() const
{
	static std::string prefix;
	prefix = _nick;
	if (!_user.empty())
		prefix += "!" + _user;
	prefix += "@" + _hostname;
	return prefix;
}

bool	Client::isRegistered() const { return registrationComplete(); }
bool	Client::isWelcomed() const { return _welcomed; }
const std::set<Channel *>	&Client::getChannels() const { return _channels; }
const std::set<std::string>	&Client::getInvitedChannels() const { return _invited; }
bool	Client::hasPendingOutput() const { return !_writeBuffer.empty(); }

/* ---------------- setters / state ---------------- */

void	Client::setNick(const std::string &nick)
{
	_nick = nick;
	_nickSet = true;
}

void	Client::setUser(const std::string &user)
{
	_user = user;
	_userSet = true;
}

void	Client::setRealName(const std::string &real) { _realname = real; }
void	Client::setPassOk(bool ok) { _passOk = ok; }
void	Client::setWelcomed(bool welcomed) { _welcomed = welcomed; }
bool	Client::passOk() const { return _passOk; }
bool	Client::nickSet() const { return _nickSet; }
bool	Client::userSet() const { return _userSet; }

bool	Client::registrationComplete() const
{
	return _passOk && _nickSet && _userSet;
}

void	Client::markConnected() { _welcomed = true; }

/* ---------------- channel membership ---------------- */

void	Client::joinChannel(Channel *ch)
{
	if (ch)
		_channels.insert(ch);
}

void	Client::leaveChannel(Channel *ch)
{
	if (ch)
		_channels.erase(ch);
}

void	Client::addInvite(const std::string &channelName)
{
	_invited.insert(channelName);
}

void	Client::removeInvite(const std::string &channelName)
{
	_invited.erase(channelName);
}

bool	Client::isInvitedTo(const std::string &channelName) const
{
	return _invited.find(channelName) != _invited.end();
}

/* ---------------- I/O buffers ---------------- */

void	Client::appendReadBuffer(const std::string &data)
{
	_readBuffer += data;
}

/*
 * If the read buffer contains one complete IRC message (ending with
 * \r\n or \n), fill cmd with it (without the line ending) and remove
 * it from the buffer. Returns true when a message was extracted.
 */
bool	Client::extractCommand(std::string &cmd)
{
	size_t	pos = _readBuffer.find("\r\n");
	size_t	len = 2;

	if (pos == std::string::npos)
	{
		pos = _readBuffer.find('\n');
		if (pos == std::string::npos)
			return false;
		len = 1;
	}
	cmd = _readBuffer.substr(0, pos);
	_readBuffer.erase(0, pos + len);
	return true;
}

void	Client::clearReadBuffer() { _readBuffer.clear(); }

void	Client::queueOutput(const std::string &msg)
{
	_writeBuffer += msg;
}

std::string	Client::takeOutput()
{
	std::string out = _writeBuffer;
	_writeBuffer.clear();
	return out;
}

bool	Client::operator==(const Client &other) const
{
	return _fd == other._fd;
}
