#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>

class Client;
class Server;

class Channel
{
public:
	Channel(const std::string &name, Client *creator);
	~Channel();

	/* --- getters --- */
	const std::string			&getName() const;
	const std::string			&getTopic() const;
	const std::string			&getTopicSetter() const;
	const std::string			&getKey() const;
	size_t						getUserLimit() const;
	bool						hasKey() const;
	bool						isInviteOnly() const;
	bool						isTopicRestricted() const;
	bool						hasUserLimit() const;
	const std::set<Client *>	&getMembers() const;
	const std::set<Client *>	&getOperators() const;
	bool						isMember(Client *c) const;
	bool						isOperator(Client *c) const;
	bool						isInvited(Client *c) const;
	size_t						memberCount() const;
	bool						isFull() const;
	std::string					getModeString() const; // "+itkol" style
	std::string					getModeArgs() const;

	/* --- setters / modes --- */
	void	setTopic(const std::string &topic, const std::string &setter);
	void	clearTopic();
	void	setKey(const std::string &key);
	void	clearKey();
	void	setInviteOnly(bool on);
	void	setTopicRestricted(bool on);
	void	setUserLimit(size_t limit);
	void	clearUserLimit();

	/* --- membership --- */
	void	addMember(Client *c);
	void	removeMember(Client *c);
	void	addOperator(Client *c);
	void	removeOperator(Client *c);
	void	addInvite(Client *c);
	void	removeInvite(Client *c);

	/* --- broadcast (Server needed for prefix formatting) --- */
	void	broadcast(Server &server, const std::string &message, Client *exclude = 0);

private:
	Channel();
	Channel(const Channel &);
	Channel &operator=(const Channel &);

	std::string				_name;
	std::string				_topic;
	std::string				_topicSetter;

	bool					_inviteOnly;      // +i
	bool					_topicRestricted; // +t
	std::string				_key;             // +k (empty = no key)
	size_t					_userLimit;       // +l (0 = no limit)

	std::set<Client *>		_members;
	std::set<Client *>		_operators;        // +o users
	std::set<Client *>		_invited;
};

#endif
