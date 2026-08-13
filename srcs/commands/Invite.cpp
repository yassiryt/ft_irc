#include "Server.hpp"
#include "Reply.hpp"

/*
 * ============================ TODO - yel-arib =============================
 * INVITE <nick> <channel>
 *
 *   1. need more params   -> ERR_NEEDMOREPARAMS
 *   2. inviter on channel -> else ERR_NOTONCHANNEL
 *   3. if +i, inviter must be operator -> else ERR_CHANOPRIVSNEEDED
 *   4. target exists      -> else ERR_NOSUCHNICK
 *   5. target not already on channel -> else ERR_USERONCHANNEL
 *   6. add invite to client + channel, send RPL_INVITING (341) to inviter
 *   7. send ":<inviter prefix> INVITE <target> :<chan>" to target
 * ==========================================================================
 */
void	Server::cmdInvite(Client &client, const std::vector<std::string> &args)
{
	(void)client;
	(void)args;
	/* TODO */
}
