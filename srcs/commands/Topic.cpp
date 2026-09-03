#include "Server.hpp"
#include "Reply.hpp"

void	Server::cmdTopic(Client &client, const std::vector<std::string> &args)
{
	if (!client.isRegistered())
	{
		sendTo(client, Reply::errNotRegistered(_serverName, client.getNick()));
		return;
	}
	if (args.empty())
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "TOPIC"));
		return;
	}

	std::string chanName = args[0];
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

	if (args.size() == 1)
	{
		if (ch->getTopic().empty())
			sendTo(client, Reply::rplNoTopic(_serverName, client.getNick(), chanName));
		else
		{
			sendTo(client, Reply::rplTopic(_serverName, client.getNick(), chanName, ch->getTopic()));
			// we can also send RPL_TOPICWHOTIME if needed, but not strictly mandatory
		}
		return;
	}

	if (ch->isTopicRestricted() && !ch->isOperator(&client))
	{
		sendTo(client, Reply::errChanOPrivsNeeded(_serverName, client.getNick(), chanName));
		return;
	}

	std::string newTopic = args[1];
	if (newTopic.empty())
		ch->clearTopic();
	else
		ch->setTopic(newTopic, client.getPrefix());

	std::string msg = ":" + client.getPrefix() + " TOPIC " + chanName + " :" + newTopic;
	ch->broadcast(*this, msg);
}
