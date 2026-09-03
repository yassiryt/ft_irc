#include "Server.hpp"
#include "Reply.hpp"

static std::vector<std::string> splitComma(const std::string &str)
{
	std::vector<std::string> result;
	size_t start = 0;
	while (start < str.length())
	{
		size_t end = str.find(',', start);
		if (end == std::string::npos)
		{
			result.push_back(str.substr(start));
			break;
		}
		result.push_back(str.substr(start, end - start));
		start = end + 1;
	}
	return result;
}

void	Server::cmdJoin(Client &client, const std::vector<std::string> &args)
{
	if (!client.isRegistered())
	{
		sendTo(client, Reply::errNotRegistered(_serverName, client.getNick()));
		return;
	}
	if (args.empty())
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "JOIN"));
		return;
	}

	std::vector<std::string> channels = splitComma(args[0]);
	std::vector<std::string> keys;
	if (args.size() > 1)
		keys = splitComma(args[1]);

	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string chanName = channels[i];
		std::string key = (i < keys.size()) ? keys[i] : "";

		if (!isValidChannelName(chanName))
		{
			sendTo(client, Reply::errNoSuchChannel(_serverName, client.getNick(), chanName));
			continue;
		}

		Channel *ch = getChannel(chanName);
		if (ch)
		{
			if (ch->isMember(&client))
				continue;
			if (ch->isInviteOnly() && !ch->isInvited(&client) && !client.isInvitedTo(chanName))
			{
				sendTo(client, Reply::errInviteOnlyChan(_serverName, client.getNick(), chanName));
				continue;
			}
			if (ch->hasKey() && ch->getKey() != key)
			{
				sendTo(client, Reply::errBadChannelKey(_serverName, client.getNick(), chanName));
				continue;
			}
			if (ch->hasUserLimit() && ch->isFull())
			{
				sendTo(client, Reply::errChannelIsFull(_serverName, client.getNick(), chanName));
				continue;
			}
			ch->addMember(&client);
		}
		else
		{
			ch = createChannel(chanName, &client);
			ch->addMember(&client);
			ch->addOperator(&client);
		}
		
		ch->removeInvite(&client);
		client.removeInvite(chanName);
		client.joinChannel(ch);

		std::string joinMsg = ":" + client.getPrefix() + " JOIN :" + chanName;
		ch->broadcast(*this, joinMsg);

		if (ch->getTopic().empty())
			sendTo(client, Reply::rplNoTopic(_serverName, client.getNick(), chanName));
		else
			sendTo(client, Reply::rplTopic(_serverName, client.getNick(), chanName, ch->getTopic()));

		std::string namesList;
		const std::set<Client*> &members = ch->getMembers();
		for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it)
		{
			if (!namesList.empty())
				namesList += " ";
			if (ch->isOperator(*it))
				namesList += "@";
			namesList += (*it)->getNick();
		}
		sendTo(client, Reply::rplNamReply(_serverName, client.getNick(), chanName, namesList));
		sendTo(client, Reply::rplEndOfNames(_serverName, client.getNick(), chanName));
	}
}
