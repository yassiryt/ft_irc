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

static void handlePrivMsgNotice(Server *serv, Client &client, const std::vector<std::string> &args, bool isNotice)
{
	if (!client.isRegistered())
	{
		if (!isNotice)
			serv->sendTo(client, Reply::errNotRegistered(serv->getServerName(), client.getNick()));
		return;
	}
	if (args.empty())
	{
		if (!isNotice)
			serv->sendTo(client, Reply::errNoRecipient(serv->getServerName(), client.getNick(), isNotice ? "NOTICE" : "PRIVMSG"));
		return;
	}
	if (args.size() == 1)
	{
		if (!isNotice)
			serv->sendTo(client, Reply::errNoTextToSend(serv->getServerName(), client.getNick()));
		return;
	}

	std::vector<std::string> targets = splitComma(args[0]);
	std::string text = args[1];

	for (size_t i = 0; i < targets.size(); ++i)
	{
		std::string target = targets[i];
		
		if (target[0] == '#' || target[0] == '&')
		{
			Channel *ch = serv->getChannel(target);
			if (!ch)
			{
				if (!isNotice)
					serv->sendTo(client, Reply::errNoSuchChannel(serv->getServerName(), client.getNick(), target));
				continue;
			}
			if (!ch->isMember(&client))
			{
				if (!isNotice)
					serv->sendTo(client, Reply::numbered(ERR_CANNOTSENDTOCHAN, serv->getServerName(), client.getNick(), target + " :Cannot send to channel"));
				continue;
			}
			std::string msg = ":" + client.getPrefix() + " " + (isNotice ? "NOTICE " : "PRIVMSG ") + target + " :" + text;
			ch->broadcast(*serv, msg, &client);
		}
		else
		{
			Client *dest = serv->getClientByNick(target);
			if (!dest)
			{
				if (!isNotice)
					serv->sendTo(client, Reply::errNoSuchNick(serv->getServerName(), client.getNick(), target));
				continue;
			}
			std::string msg = ":" + client.getPrefix() + " " + (isNotice ? "NOTICE " : "PRIVMSG ") + target + " :" + text;
			serv->sendTo(*dest, msg);
		}
	}
}

void	Server::cmdPrivMsg(Client &client, const std::vector<std::string> &args)
{
	handlePrivMsgNotice(this, client, args, false);
}

void	Server::cmdNotice(Client &client, const std::vector<std::string> &args)
{
	handlePrivMsgNotice(this, client, args, true);
}
