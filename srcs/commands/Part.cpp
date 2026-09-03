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

void	Server::cmdPart(Client &client, const std::vector<std::string> &args)
{
	if (!client.isRegistered())
	{
		sendTo(client, Reply::errNotRegistered(_serverName, client.getNick()));
		return;
	}
	if (args.empty())
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "PART"));
		return;
	}

	std::vector<std::string> channels = splitComma(args[0]);
	std::string reason = "";
	if (args.size() > 1)
		reason = args[1];

	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string chanName = channels[i];
		Channel *ch = getChannel(chanName);

		if (!ch)
		{
			sendTo(client, Reply::errNoSuchChannel(_serverName, client.getNick(), chanName));
			continue;
		}
		if (!ch->isMember(&client))
		{
			sendTo(client, Reply::errNotOnChannel(_serverName, client.getNick(), chanName));
			continue;
		}

		std::string partMsg = ":" + client.getPrefix() + " PART " + chanName;
		if (!reason.empty())
			partMsg += " :" + reason;
		
		ch->broadcast(*this, partMsg);
		ch->removeMember(&client);
		client.leaveChannel(ch);

		if (ch->memberCount() == 0)
			removeChannel(ch);
	}
}
