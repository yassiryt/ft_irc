#include "Server.hpp"
#include "Reply.hpp"
#include <cstdlib>
#include <sstream>

void	Server::cmdMode(Client &client, const std::vector<std::string> &args)
{
	if (!client.isRegistered())
	{
		sendTo(client, Reply::errNotRegistered(_serverName, client.getNick()));
		return;
	}
	if (args.empty())
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "MODE"));
		return;
	}

	std::string target = args[0];
	if (target[0] != '#' && target[0] != '&')
	{
		if (target == client.getNick() && args.size() == 1)
			sendTo(client, Reply::numbered(RPL_UMODEIS, _serverName, client.getNick(), "+"));
		else if (target != client.getNick())
			sendTo(client, Reply::numbered(ERR_USERSDONTMATCH, _serverName, client.getNick(), ":Cannot change mode for other users"));
		return;
	}

	Channel *ch = getChannel(target);
	if (!ch)
	{
		sendTo(client, Reply::errNoSuchChannel(_serverName, client.getNick(), target));
		return;
	}

	if (args.size() == 1)
	{
		std::string modes = ch->getModeString();
		std::string modeArgs = ch->getModeArgs();
		sendTo(client, Reply::numbered(RPL_CHANNELMODEIS, _serverName, client.getNick(), target + " " + modes + (modeArgs.empty() ? "" : " " + modeArgs)));
		return;
	}

	if (!ch->isOperator(&client))
	{
		sendTo(client, Reply::errChanOPrivsNeeded(_serverName, client.getNick(), target));
		return;
	}

	std::string modeString = args[1];
	bool adding = true;
	size_t argIdx = 2;

	std::string appliedModes = "";
	std::string appliedArgs = "";
	char lastSign = '\0';

	for (size_t i = 0; i < modeString.length(); ++i)
	{
		char c = modeString[i];
		if (c == '+')
		{
			adding = true;
			continue;
		}
		if (c == '-')
		{
			adding = false;
			continue;
		}

		bool changed = false;
		std::string argStr = "";

		switch (c)
		{
			case 'i':
				if (ch->isInviteOnly() != adding)
				{
					ch->setInviteOnly(adding);
					changed = true;
				}
				break;
			case 't':
				if (ch->isTopicRestricted() != adding)
				{
					ch->setTopicRestricted(adding);
					changed = true;
				}
				break;
			case 'k':
				if (adding)
				{
					if (argIdx < args.size())
					{
						argStr = args[argIdx++];
						if (!ch->hasKey() || ch->getKey() != argStr)
						{
							ch->setKey(argStr);
							changed = true;
						}
					}
				}
				else
				{
					if (argIdx < args.size())
					{
						argStr = args[argIdx++];
						if (ch->hasKey() && ch->getKey() == argStr)
						{
							ch->clearKey();
							changed = true;
						}
					}
					else if (ch->hasKey())
					{
						ch->clearKey();
						changed = true;
					}
				}
				break;
			case 'o':
				if (argIdx < args.size())
				{
					argStr = args[argIdx++];
					Client *targetClient = getClientByNick(argStr);
					if (!targetClient || !ch->isMember(targetClient))
					{
						sendTo(client, Reply::errUserNotInChannel(_serverName, client.getNick(), argStr, target));
						continue;
					}
					if (adding)
					{
						if (!ch->isOperator(targetClient))
						{
							ch->addOperator(targetClient);
							changed = true;
						}
					}
					else
					{
						if (ch->isOperator(targetClient))
						{
							ch->removeOperator(targetClient);
							changed = true;
						}
					}
				}
				break;
			case 'l':
				if (adding)
				{
					if (argIdx < args.size())
					{
						argStr = args[argIdx++];
						long limit = std::atol(argStr.c_str());
						if (limit > 0)
						{
							if (!ch->hasUserLimit() || ch->getUserLimit() != (size_t)limit)
							{
								ch->setUserLimit(limit);
								changed = true;
							}
						}
					}
				}
				else
				{
					if (ch->hasUserLimit())
					{
						ch->clearUserLimit();
						changed = true;
					}
				}
				break;
			default:
				sendTo(client, Reply::numbered(ERR_UNKNOWNMODE, _serverName, client.getNick(), std::string(1, c) + " :is unknown mode char to me"));
				break;
		}

		if (changed)
		{
			char currentSign = adding ? '+' : '-';
			if (lastSign != currentSign)
			{
				appliedModes += currentSign;
				lastSign = currentSign;
			}
			appliedModes += c;
			if (!argStr.empty())
			{
				if (!appliedArgs.empty())
					appliedArgs += " ";
				appliedArgs += argStr;
			}
		}
	}

	if (!appliedModes.empty())
	{
		std::string msg = ":" + client.getPrefix() + " MODE " + target + " " + appliedModes;
		if (!appliedArgs.empty())
			msg += " " + appliedArgs;
		ch->broadcast(*this, msg);
	}
}
