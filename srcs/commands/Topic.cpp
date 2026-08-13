#include "Server.hpp"
#include "Reply.hpp"

/*
 * ============================ TODO - yel-arib =============================
 * TOPIC <channel> [:<new topic>]
 *
 *   - only <channel>:      show topic (RPL_TOPIC 332 / RPL_NOTOPIC 331)
 *                          must be a member -> else ERR_NOTONCHANNEL
 *   - with <new topic>:
 *        - if +t, must be operator -> else ERR_CHANOPRIVSNEEDED
 *        - empty new topic clears it
 *        - broadcast ":<prefix> TOPIC <chan> :<topic>" to all members
 * ==========================================================================
 */
void	Server::cmdTopic(Client &client, const std::vector<std::string> &args)
{
	(void)client;
	(void)args;
	/* TODO */
}
