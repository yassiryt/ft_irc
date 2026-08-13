#ifndef REPLY_HPP
#define REPLY_HPP

#include <string>
#include <sstream>

/*
 * Numeric replies per RFC 1459 / RFC 2812.
 * reply() helpers build ":<server> <numeric> <nick> <rest>\r\n"
 */

#define RPL_WELCOME			001
#define RPL_YOURHOST		002
#define RPL_CREATED			003
#define RPL_MYINFO			004
#define RPL_BOUNCE			005

#define ERR_NOSUCHNICK		401
#define ERR_NOSUCHCHANNEL	403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_NORECIPIENT		411
#define ERR_NOTEXTTOSEND	412
#define ERR_UNKNOWNCOMMAND	421
#define ERR_NOMOTD			422
#define ERR_NONICKNAMEGIVEN	431
#define ERR_ERRONEUSNICKNAME 432
#define ERR_NICKNAMEINUSE	433
#define ERR_USERNOTINCHANNEL 441
#define ERR_NOTONCHANNEL	442
#define ERR_USERONCHANNEL	443
#define ERR_NOTREGISTERED	451
#define ERR_NEEDMOREPARAMS	461
#define ERR_ALREADYREGISTRED 462
#define ERR_PASSWDMISMATCH	464
#define ERR_CHANNELISFULL	471
#define ERR_UNKNOWNMODE		472
#define ERR_INVITEONLYCHAN	473
#define ERR_BANNEDFROMCHAN	474
#define ERR_BADCHANNELKEY	475
#define ERR_BADCHANMASK		476
#define ERR_CHANOPRIVSNEEDED 482
#define ERR_UMODEUNKNOWNFLAG 501
#define ERR_USERSDONTMATCH	502

#define RPL_UMODEIS			221
#define RPL_CHANNELMODEIS	324
#define RPL_NOTOPIC			331
#define RPL_TOPIC			332
#define RPL_TOPICWHOTIME	333
#define RPL_INVITING		341
#define RPL_NAMREPLY		353
#define RPL_ENDOFNAMES		366
#define RPL_MOTD			372
#define RPL_ENDOFMOTD		376

namespace Reply
{
	/* low-level: "<server> <numeric> <target> [params]" */
	std::string	numeric(const std::string &serverName, int code,
						const std::string &target, const std::string &params);

	/* convenience: build a numbered reply with the given text */
	std::string	numbered(int code, const std::string &serverName,
						 const std::string &nick, const std::string &rest);

	/* error helpers used by the commands */
	std::string	errUnknownCommand(const std::string &serverName,
								  const std::string &nick, const std::string &cmd);
	std::string	errNotRegistered(const std::string &serverName, const std::string &nick);
	std::string	errNeedMoreParams(const std::string &serverName,
								   const std::string &nick, const std::string &cmd);
	std::string	errNoSuchNick(const std::string &serverName,
							  const std::string &nick, const std::string &target);
	std::string	errNoSuchChannel(const std::string &serverName,
								 const std::string &nick, const std::string &channel);
	std::string	errNotOnChannel(const std::string &serverName,
								const std::string &nick, const std::string &channel);
	std::string	errNoRecipient(const std::string &serverName, const std::string &nick,
							   const std::string &cmd);
	std::string	errNoTextToSend(const std::string &serverName, const std::string &nick);
	std::string	errChanOPrivsNeeded(const std::string &serverName,
									const std::string &nick, const std::string &channel);
	std::string	errUserNotInChannel(const std::string &serverName, const std::string &nick,
									 const std::string &target, const std::string &channel);
	std::string	errNicknameInUse(const std::string &serverName, const std::string &nick);
	std::string	errErroneusNickname(const std::string &serverName, const std::string &nick);
	std::string	errNoNicknameGiven(const std::string &serverName);
	std::string	errPasswordMismatch(const std::string &serverName);
	std::string	errAlreadyRegistered(const std::string &serverName, const std::string &nick);
	std::string	errInviteOnlyChan(const std::string &serverName, const std::string &nick,
								  const std::string &channel);
	std::string	errBadChannelKey(const std::string &serverName, const std::string &nick,
								 const std::string &channel);
	std::string	errChannelIsFull(const std::string &serverName, const std::string &nick,
								 const std::string &channel);

	/* rpl helpers */
	std::string	rplWelcome(const std::string &serverName, const std::string &nick,
						   const std::string &user, const std::string &host);
	std::string	rplTopic(const std::string &serverName, const std::string &nick,
						 const std::string &channel, const std::string &topic);
	std::string	rplNoTopic(const std::string &serverName, const std::string &nick,
						   const std::string &channel);
	std::string	rplNamReply(const std::string &serverName, const std::string &nick,
							const std::string &channel, const std::string &names);
	std::string	rplEndOfNames(const std::string &serverName, const std::string &nick,
							  const std::string &channel);
}

/* utility: convert anything streamable to string (C++98 friendly) */
template <typename T>
std::string toStr(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

#endif
