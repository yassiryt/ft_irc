#include "Server.hpp"
#include "Reply.hpp"

void	Server::cmdKick(Client &client, const std::vector<std::string> &args)
{
	if (!client.isRegistered())
	{
		sendTo(client, Reply::errNotRegistered(_serverName, client.getNick()));
		return;
	}
	if (args.size() < 2)
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "KICK"));
		return;
	}

	std::string chanName = args[0];
	std::string targetNick = args[1];
	std::string reason = "";
	if (args.size() > 2)
		reason = args[2];

	Channel *ch = getChannel(chanName);
	if (!ch)
	{
		sendTo(client, Reply::errNoSuchChannel(_serverName, client.getNick(), chanName));
		return;
	}

	if (!ch->isMember(&client))
	{
		sendTo(client, Reply::errNotOnChannel(_serverName, client.getNick(), chanName));
		return;
	}

	if (!ch->isOperator(&client))
	{
		sendTo(client, Reply::errChanOPrivsNeeded(_serverName, client.getNick(), chanName));
		return;
	}

	Client *target = getClientByNick(targetNick);
	if (!target || !ch->isMember(target))
	{
		sendTo(client, Reply::errUserNotInChannel(_serverName, client.getNick(), targetNick, chanName));
		return;
	}

	std::string kickMsg = ":" + client.getPrefix() + " KICK " + chanName + " " + targetNick;
	if (!reason.empty())
		kickMsg += " :" + reason;
	else
		kickMsg += " :" + client.getNick(); // default reason

	ch->broadcast(*this, kickMsg);
	ch->removeMember(target);
	target->leaveChannel(ch);

	if (ch->memberCount() == 0)
		removeChannel(ch);
}
