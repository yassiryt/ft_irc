#include "Server.hpp"
#include "Reply.hpp"

/*
 * ============================ TODO - yatanagh =============================
 * PRIVMSG <target>{,<target>} :<text>
 *
 *   - no target            -> ERR_NORECIPIENT
 *   - no text              -> ERR_NOTEXTTOSEND
 *   - target == channel:   forward to all members except sender
 *                          (sender not on channel -> ERR_CANNOTSENDTOCHAN)
 *   - target == nick:      forward to that client (not found -> ERR_NOSUCHNICK)
 *
 * NOTICE: same as PRIVMSG but NEVER send any error reply.
 * ==========================================================================
 */
void	Server::cmdPrivMsg(Client &client, const std::vector<std::string> &args)
{
	(void)client;
	(void)args;
	/* TODO */
}

void	Server::cmdNotice(Client &client, const std::vector<std::string> &args)
{
	(void)client;
	(void)args;
	/* TODO */
}
