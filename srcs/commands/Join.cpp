#include "Server.hpp"
#include "Reply.hpp"

/*
 * ============================ TODO - yatanagh =============================
 * JOIN <channel>{,<channel>} [<key>{,<key>}]
 *
 * Flow for each channel in the list:
 *   1. check registration  -> ERR_NOTREGISTERED
 *   2. validate name       -> ERR_NOSUCHCHANNEL
 *   3. if channel exists:
 *        - already a member? ignore
 *        - +i and not invited?          -> ERR_INVITEONLYCHAN
 *        - +k and key missing/wrong?    -> ERR_BADCHANNELKEY
 *        - +l and full?                 -> ERR_CHANNELISFULL
 *   4. else create it (creator becomes operator)
 *   5. add member, broadcast ":<prefix> JOIN :<chan>"
 *   6. send topic (332/331) + names (353/366)
 * ==========================================================================
 */
void	Server::cmdJoin(Client &client, const std::vector<std::string> &args)
{
	if (args.empty())
	{
		sendTo(client, Reply::errNeedMoreParams(_serverName, client.getNick(), "JOIN"));
		return;
	}
	(void)client;
	(void)args;
	/* TODO */
}
