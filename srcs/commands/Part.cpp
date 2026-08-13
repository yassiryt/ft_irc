#include "Server.hpp"
#include "Reply.hpp"

/*
 * ============================ TODO - yel-arib =============================
 * PART <channel>{,<channel>} [:<reason>]
 *
 *   1. check registration -> ERR_NOTREGISTERED
 *   2. channel must exist  -> ERR_NOSUCHCHANNEL
 *   3. must be a member    -> ERR_NOTONCHANNEL
 *   4. broadcast ":<prefix> PART <chan> [:<reason>]" to all members
 *   5. remove member; delete channel if empty
 * ==========================================================================
 */
void	Server::cmdPart(Client &client, const std::vector<std::string> &args)
{
	(void)client;
	(void)args;
	/* TODO */
}
