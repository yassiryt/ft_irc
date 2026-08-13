#include "Reply.hpp"

namespace Reply
{

std::string	numeric(const std::string &serverName, int code,
					const std::string &target, const std::string &params)
{
	std::string out = ":" + serverName + " " + toStr(code) + " " + target;
	if (!params.empty())
		out += " " + params;
	return out + "\r\n";
}

std::string	numbered(int code, const std::string &serverName,
					 const std::string &nick, const std::string &rest)
{
	return numeric(serverName, code, nick.empty() ? "*" : nick, rest);
}

/* ---------------- errors ---------------- */

std::string	errUnknownCommand(const std::string &serverName,
							  const std::string &nick, const std::string &cmd)
{
	return numbered(ERR_UNKNOWNCOMMAND, serverName, nick, cmd + " :Unknown command");
}

std::string	errNotRegistered(const std::string &serverName, const std::string &nick)
{
	return numbered(ERR_NOTREGISTERED, serverName, nick, ":You have not registered");
}

std::string	errNeedMoreParams(const std::string &serverName,
							   const std::string &nick, const std::string &cmd)
{
	return numbered(ERR_NEEDMOREPARAMS, serverName, nick, cmd + " :Not enough parameters");
}

std::string	errNoSuchNick(const std::string &serverName,
						  const std::string &nick, const std::string &target)
{
	return numbered(ERR_NOSUCHNICK, serverName, nick, target + " :No such nick/channel");
}

std::string	errNoSuchChannel(const std::string &serverName,
							 const std::string &nick, const std::string &channel)
{
	return numbered(ERR_NOSUCHCHANNEL, serverName, nick, channel + " :No such channel");
}

std::string	errNotOnChannel(const std::string &serverName,
							const std::string &nick, const std::string &channel)
{
	return numbered(ERR_NOTONCHANNEL, serverName, nick, channel + " :You're not on that channel");
}

std::string	errNoRecipient(const std::string &serverName, const std::string &nick,
						   const std::string &cmd)
{
	return numbered(ERR_NORECIPIENT, serverName, nick, ":No recipient given (" + cmd + ")");
}

std::string	errNoTextToSend(const std::string &serverName, const std::string &nick)
{
	return numbered(ERR_NOTEXTTOSEND, serverName, nick, ":No text to send");
}

std::string	errChanOPrivsNeeded(const std::string &serverName,
								const std::string &nick, const std::string &channel)
{
	return numbered(ERR_CHANOPRIVSNEEDED, serverName, nick,
					channel + " :You're not channel operator");
}

std::string	errUserNotInChannel(const std::string &serverName, const std::string &nick,
								 const std::string &target, const std::string &channel)
{
	return numbered(ERR_USERNOTINCHANNEL, serverName, nick,
					target + " " + channel + " :They aren't on that channel");
}

std::string	errNicknameInUse(const std::string &serverName, const std::string &nick)
{
	return numbered(ERR_NICKNAMEINUSE, serverName, nick.empty() ? "*" : nick,
					nick + " :Nickname is already in use");
}

std::string	errErroneusNickname(const std::string &serverName, const std::string &nick)
{
	return numbered(ERR_ERRONEUSNICKNAME, serverName, nick.empty() ? "*" : nick,
					nick + " :Erroneous nickname");
}

std::string	errNoNicknameGiven(const std::string &serverName)
{
	return numbered(ERR_NONICKNAMEGIVEN, serverName, "*", ":No nickname given");
}

std::string	errPasswordMismatch(const std::string &serverName)
{
	return numbered(ERR_PASSWDMISMATCH, serverName, "*", ":Password incorrect");
}

std::string	errAlreadyRegistered(const std::string &serverName, const std::string &nick)
{
	return numbered(ERR_ALREADYREGISTRED, serverName, nick, ":You may not reregister");
}

std::string	errInviteOnlyChan(const std::string &serverName, const std::string &nick,
							  const std::string &channel)
{
	return numbered(ERR_INVITEONLYCHAN, serverName, nick,
					channel + " :Cannot join channel (+i)");
}

std::string	errBadChannelKey(const std::string &serverName, const std::string &nick,
							 const std::string &channel)
{
	return numbered(ERR_BADCHANNELKEY, serverName, nick,
					channel + " :Cannot join channel (+k)");
}

std::string	errChannelIsFull(const std::string &serverName, const std::string &nick,
							 const std::string &channel)
{
	return numbered(ERR_CHANNELISFULL, serverName, nick,
					channel + " :Cannot join channel (+l)");
}

/* ---------------- replies ---------------- */

std::string	rplWelcome(const std::string &serverName, const std::string &nick,
					   const std::string &user, const std::string &host)
{
	return numbered(RPL_WELCOME, serverName, nick,
					":Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host);
}

std::string	rplTopic(const std::string &serverName, const std::string &nick,
					 const std::string &channel, const std::string &topic)
{
	return numbered(RPL_TOPIC, serverName, nick, channel + " :" + topic);
}

std::string	rplNoTopic(const std::string &serverName, const std::string &nick,
					   const std::string &channel)
{
	return numbered(RPL_NOTOPIC, serverName, nick, channel + " :No topic is set");
}

std::string	rplNamReply(const std::string &serverName, const std::string &nick,
						const std::string &channel, const std::string &names)
{
	return numbered(RPL_NAMREPLY, serverName, nick, "= " + channel + " :" + names);
}

std::string	rplEndOfNames(const std::string &serverName, const std::string &nick,
						  const std::string &channel)
{
	return numbered(RPL_ENDOFNAMES, serverName, nick, channel + " :End of /NAMES list");
}

}
