#include "Server.hpp"
#include "Reply.hpp"

void	Server::cmdInvite(Client &client, const std::vector<std::string> &args)
{
	if (!client.isRegistered())
	{
		sendTo(client, Reply::errNotRegistered(_serverName, client.getNick()));
		return;
	}
	if (args.size() < 2)
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "INVITE"));
		return;
	}

	std::string targetNick = args[0];
	std::string chanName = args[1];

	Channel *ch = getChannel(chanName);
	if (ch)
	{
		if (!ch->isMember(&client))
		{
			sendTo(client, Reply::errNotOnChannel(_serverName, client.getNick(), chanName));
			return;
		}

		if (ch->isInviteOnly() && !ch->isOperator(&client))
		{
			sendTo(client, Reply::errChanOPrivsNeeded(_serverName, client.getNick(), chanName));
			return;
		}
	}

	Client *target = getClientByNick(targetNick);
	if (!target)
	{
		sendTo(client, Reply::errNoSuchNick(_serverName, client.getNick(), targetNick));
		return;
	}

	if (ch && ch->isMember(target))
	{
		sendTo(client, Reply::numbered(ERR_USERONCHANNEL, _serverName, client.getNick(), targetNick + " " + chanName + " :is already on channel"));
		return;
	}

	if (ch)
		ch->addInvite(target);
	else
		target->addInvite(chanName); // If channel doesn't exist yet, we just add the invite to the client

	sendTo(client, Reply::numbered(RPL_INVITING, _serverName, client.getNick(), targetNick + " " + chanName));

	std::string msg = ":" + client.getPrefix() + " INVITE " + targetNick + " :" + chanName;
	sendTo(*target, msg);
}
