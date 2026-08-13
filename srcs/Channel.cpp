#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Reply.hpp"

Channel::Channel(const std::string &name, Client *creator)
	: _name(name), _inviteOnly(false), _topicRestricted(false),
	  _userLimit(0)
{
	(void)creator;
}

Channel::~Channel()
{
}

/* ---------------- getters ---------------- */

const std::string			&Channel::getName() const { return _name; }
const std::string			&Channel::getTopic() const { return _topic; }
const std::string			&Channel::getTopicSetter() const { return _topicSetter; }
const std::string			&Channel::getKey() const { return _key; }
size_t						Channel::getUserLimit() const { return _userLimit; }
bool						Channel::hasKey() const { return !_key.empty(); }
bool						Channel::isInviteOnly() const { return _inviteOnly; }
bool						Channel::isTopicRestricted() const { return _topicRestricted; }
bool						Channel::hasUserLimit() const { return _userLimit > 0; }
const std::set<Client *>	&Channel::getMembers() const { return _members; }
const std::set<Client *>	&Channel::getOperators() const { return _operators; }
bool						Channel::isMember(Client *c) const { return _members.find(c) != _members.end(); }
bool						Channel::isOperator(Client *c) const { return _operators.find(c) != _operators.end(); }
bool						Channel::isInvited(Client *c) const { return _invited.find(c) != _invited.end(); }
size_t						Channel::memberCount() const { return _members.size(); }
bool						Channel::isFull() const { return _userLimit > 0 && _members.size() >= _userLimit; }

std::string	Channel::getModeString() const
{
	std::string modes = "+";
	if (_inviteOnly)		modes += "i";
	if (_topicRestricted)	modes += "t";
	if (!_key.empty())		modes += "k";
	if (_userLimit > 0)		modes += "l";
	return modes;
}

std::string	Channel::getModeArgs() const
{
	std::string args;
	if (!_key.empty())
		args += _key + " ";
	if (_userLimit > 0)
		args += " " + toStr(_userLimit);
	return args;
}

/* ---------------- setters / modes ---------------- */

void	Channel::setTopic(const std::string &topic, const std::string &setter)
{
	_topic = topic;
	_topicSetter = setter;
}

void	Channel::clearTopic()
{
	_topic.clear();
	_topicSetter.clear();
}

void	Channel::setKey(const std::string &key) { _key = key; }
void	Channel::clearKey() { _key.clear(); }
void	Channel::setInviteOnly(bool on) { _inviteOnly = on; }
void	Channel::setTopicRestricted(bool on) { _topicRestricted = on; }
void	Channel::setUserLimit(size_t limit) { _userLimit = limit; }
void	Channel::clearUserLimit() { _userLimit = 0; }

/* ---------------- membership ---------------- */

void	Channel::addMember(Client *c)
{
	if (c)
		_members.insert(c);
}

void	Channel::removeMember(Client *c)
{
	if (!c)
		return;
	_members.erase(c);
	removeOperator(c);
	removeInvite(c);
}

void	Channel::addOperator(Client *c)
{
	if (c)
		_operators.insert(c);
}

void	Channel::removeOperator(Client *c)
{
	if (c)
		_operators.erase(c);
}

void	Channel::addInvite(Client *c)
{
	if (c)
		_invited.insert(c);
}

void	Channel::removeInvite(Client *c)
{
	if (c)
		_invited.erase(c);
}

/* ---------------- broadcast ---------------- */

void	Channel::broadcast(Server &server, const std::string &message, Client *exclude)
{
	std::set<Client *>::iterator it;
	for (it = _members.begin(); it != _members.end(); ++it)
	{
		if (*it != exclude)
			server.sendTo(**it, message);
	}
}
