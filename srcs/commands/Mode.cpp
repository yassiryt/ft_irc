#include "Server.hpp"
#include "Reply.hpp"

#include <cstdlib>

/*
 * ============================ TODO - yel-arib =============================
 * MODE <channel>                    -> show modes (RPL_CHANNELMODEIS 324)
 * MODE <channel> <modes> [<args>]   -> set modes (channel operator only)
 *
 * Supported mode flags:
 *   +i / -i  invite-only
 *   +t / -t  topic restricted to operators
 *   +k <key> / -k   channel key (password)
 *   +o <nick> / -o <nick>  give/take channel operator
 *   +l <limit> / -l  user limit
 *
 * Checks:
 *   - channel exists            -> ERR_NOSUCHCHANNEL
 *   - no modes given            -> just show current modes
 *   - caller is operator        -> ERR_CHANOPRIVSNEEDED
 *   - nick not on channel (+o)  -> ERR_USERNOTINCHANNEL
 *   - unknown flag              -> ERR_UNKNOWNMODE
 *   - broadcast every change: ":<prefix> MODE <chan> <flag> [<arg>]"
 * ==========================================================================
 */
void	Server::cmdMode(Client &client, const std::vector<std::string> &args)
{
	if (args.empty())
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "MODE"));
		return;
	}
	(void)client;
	(void)args;
	/* TODO */
}
