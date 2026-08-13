#include "Server.hpp"
#include "Reply.hpp"

/*
 * ============================ TODO - yatanagh =============================
 * KICK <channel> <user> [:<reason>]     (channel operator only)
 *
 *   1. need more params   -> ERR_NEEDMOREPARAMS
 *   2. channel must exist  -> ERR_NOSUCHCHANNEL
 *   3. kicker is operator  -> else ERR_CHANOPRIVSNEEDED
 *   4. target on channel   -> else ERR_USERNOTINCHANNEL
 *   5. broadcast ":<kicker prefix> KICK <chan> <target> [:<reason>]"
 *   6. remove target from channel
 * ==========================================================================
 */
void	Server::cmdKick(Client &client, const std::vector<std::string> &args)
{
	(void)client;
	(void)args;
	/* TODO */
}
