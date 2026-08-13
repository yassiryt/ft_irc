# yatanagh — Day-by-day guide to evaluation (JOIN / PART / PRIVMSG / KICK)

Your scope: channel membership and messaging. Your files:
`srcs/commands/Join.cpp`, `Part.cpp`, `PrivMsg.cpp`, `Kick.cpp` (+ shared files, agreed with yel-arib).

Every day: `make re` before pushing, test with BOTH `nc -C` and irssi.
Start the server: `./ircserv 6667 pass`. Open irssi: `irssi -c 127.0.0.1 -p 6667 -w pass`.
Type your commands in irssi with `/join #x`, `/msg`, `/part`, `/kick`.

---

## DAY 1 — Learn the IRC message format + implement JOIN (single channel)

**Learn**
- Read RFC 2812 §3.2.1 (JOIN) and §3.1 (message format: `<command> <params> :<trailing>`).
- Re-read `srcs/Server.cpp`: `splitLine()`, `dispatch()`, `handleCommand()` — trace one command
  from socket to handler. Re-read `Channel.hpp` and `Server::sendTo`.
- Understand: `:prefix COMMAND params :trailing\r\n` — every broadcast you write must look like this.

**Build** (`Join.cpp`, TODO block at top lists the errors)
1. Guard: `if (!client.registrationComplete())` → send `ERR_NOTREGISTERED` and return.
2. Guard: `isValidChannelName(args[0])` → else `ERR_NOSUCHCHANNEL`.
3. `Channel *ch = getChannel(name)`:
   - exists + `client.getChannels()` contains it → ignore (silent return).
   - exists + `ch->isInviteOnly()` and not invited → `ERR_INVITEONLYCHAN`.
   - exists + `ch->hasKey()` and no/wrong key → `ERR_BADCHANNELKEY`.
   - exists + `ch->isFull()` → `ERR_CHANNELISFULL`.
   - else `createChannel(name, &client)` then `ch->addOperator(&client)`.
4. `ch->addMember(&client)`, `client.joinChannel(ch)`, `client.removeInvite(name)`.
5. Broadcast to ALL members (including self): `:<client.getPrefix()> JOIN :<name>`.
6. Topic: if `!ch->getTopic().empty()` → `Reply::rplTopic`, else `Reply::rplNoTopic`.
7. Names: build string `@opNick nick2` then `Reply::rplNamReply` + `Reply::rplEndOfNames`.

**Test**
- `nc -C`: PASS/NICK/USER then `JOIN #x` → expect JOIN echo + 332/331 + 353 + 366.
- Second nc joins same channel → first client sees the JOIN broadcast.

---

## DAY 2 — JOIN multi-channel + keys

**Learn**
- How `splitLine()` keeps the trailing param as one token. `args` = tokens after the command.
- RFC 2812 §3.2.1: `JOIN #a,#b k1,k2` — split both lists on `,`.

**Build**
1. Write a small local helper `splitComma(str)` returning `std::vector<std::string>`.
2. Split `args[0]` into channel list; if `args.size() > 1` split `args[1]` into key list.
3. Loop channels, use `keys[i]` for the i-th channel, extract day-1 logic into a
   `joinOne(client, name, key)` helper.
4. Careful: first-joiner-creates rule stays per-channel.

**Test**
- `JOIN #a,#b` → two echoes. `JOIN #a,#b x,y` with #a keyed `x` → works if keys align.
- `JOIN #a` with wrong key → 475. `JOIN #a` with no key → 475.
- Empty channel after all part/quit is deleted (re-join creates it fresh).

---

## DAY 3 — PRIVMSG + NOTICE

**Learn**
- RFC 2812 §3.3.1 (PRIVMSG) and §3.3.2 (NOTICE: identical, but NEVER any error reply).
- Channel vs nick targets. `Server::getClientByNick()`.

**Build** (`PrivMsg.cpp`)
1. `args.size() < 1` → `ERR_NORECIPIENT`; `< 2` or empty text → `ERR_NOTEXTTOSEND`.
2. Target starts with `#` (channel):
   - `getChannel(target)` null → `ERR_NOSUCHNICK` (RFC says no such nick/channel).
   - sender not member → `ERR_CANNOTSENDTOCHAN`.
   - else `broadcastToChannel(ch, ":<prefix> PRIVMSG <chan> :<text>", &client)`.
3. Else nick target: not found → `ERR_NOSUCHNICK`; found → `sendTo(*target, ":<prefix> PRIVMSG <nick> :<text>")`.
4. NOTICE: same routing, zero errors — just return silently on failures.
5. Support `PRIVMSG nick hello world` without colon: join `args[1..]` with spaces.

**Test**
- irssi: `/msg nick hi`, `/msg #chan hi` — verify both directions and no reply leaks to sender.
- `PRIVMSG` (no args) → 411, `PRIVMSG nick` → 412, `PRIVMSG ghost hi` → 401.
- NOTICE to wrong nick → NO error at all.

---

## DAY 4 — PART + verify QUIT broadcast

**Learn**
- RFC 2812 §3.2.2 (PART). Empty-channel deletion rule: no members → delete channel object
  AND remove from `Server::_channels` (same pattern as `Server::disconnectClient`).

**Build** (`Part.cpp`)
1. Need params → `ERR_NEEDMOREPARAMS`; channel not found → `ERR_NOSUCHCHANNEL`;
   not member → `ERR_NOTONCHANNEL`.
2. Broadcast `:<prefix> PART <chan> [:<reason>]` to ALL members (including self).
3. `ch->removeMember(&client)`, `client.leaveChannel(ch)`.
4. If `ch->memberCount() == 0` → `_channels.erase(name)` + `delete ch`.
5. Multi-channel: `PART #a,#b` with one reason for all.

**Test**
- irssi: `/part #x goodbye` — everyone sees it. Re-join #x after empty → topic is gone
  (channel was recreated). QUIT while in 2 channels → both channels get the QUIT (verify this — code already exists in `disconnectClient`, report to yel-arib if broken).

---

## DAY 5 — MILESTONE 1: pair session

**Test with yel-arib (he has MODE done partially)**
1. Two irssi users + one nc: join, talk, part, re-join.
2. Modes block you correctly: `+i` (you can't join without invite), `+k` (wrong key fails),
   `+l` (full channel fails).
3. Every error path from days 1-4, written in a checklist. Anything broken = fixed before Day 6.

---

## DAY 6 — KICK

**Learn**
- RFC 2812 §3.2.8 (KICK). Channel operator privilege check: `ch->isOperator(&client)`.

**Build** (`Kick.cpp`)
1. `args.size() < 2` → `ERR_NEEDMOREPARAMS`; channel null → `ERR_NOSUCHCHANNEL`.
2. `!ch->isOperator(&client)` → `ERR_CHANOPRIVSNEEDED`.
3. Target nick null → `ERR_NOSUCHNICK`; `!ch->isMember(target)` → `ERR_USERNOTINCHANNEL`.
4. Broadcast to ALL members: `:<prefix> KICK <chan> <target> [:<reason>]`.
5. `ch->removeMember(target)`, `target->leaveChannel(ch)`; delete channel if empty.

**Test**
- irssi: op kicks user → user sees it, user is out. Non-op kicks → 482.
- KICK ghost, KICK without reason, KICK from non-member (482 first? order per checklist).

---

## DAY 7 — Invite-aware JOIN (pair yel-arib's INVITE)

**Build**
- INVITE is yel-arib's file. You integrate: when `+i` join is attempted, check
  `client.isInvitedTo(name)` AND `ch->isInvited(&client)`; on success `removeInvite` both sides.
- Agree on who adds the invite on JOIN if already invited: member auto-accepted.

**Test** (together)
- `+i` channel: uninvited → 473. After `/invite`, join succeeds. Re-join after part → still invited? No — invite is consumed. Test both.

---

## DAY 8 — Empty-channel cleanup audit + PRIVMSG edge cases

**Build**
- Audit: every path that removes a member (PART, KICK, QUIT, disconnect) deletes empty channels.
- PRIVMSG/NOTICE to empty-string target, target = self, text with `:` inside, 200-char messages.

**Test**
- Script: join 3, kick 2, part 1, check server doesn't crash on `JOIN #x` after (recreated).

---

## DAY 9 — Stress + slow clients

**Build**
- Nothing new; fix whatever breaks.

**Test**
- 10 nc clients join same channel, one sends 100 messages → all 100 delivered to all 9.
- Slow sender: python client sending 1 byte every 0.2s → server aggregates and executes
  correctly (your read buffer handles this).
- Kill a client mid-send (Ctrl-C the nc) → no crash, others unaffected.

---

## DAY 10 — MILESTONE 2: full scenario

**Test (with yel-arib)**
Run the whole eval checklist for YOUR commands + his, on irssi. All green → move on.

---

## DAY 11 — Partial data + packet fuzzing

**Learn**
- The subject's exact nc test: `com` Ctrl+D `man` Ctrl+D `d\n` must execute one command.
  Re-read `Client::extractCommand` until you can explain it in your sleep (defense question!).

**Test**
- Split `JOIN #x` into 5 pieces → works. 10 commands in ONE packet → all execute in order.
- `\r\n` and `\n` endings mixed. Garbage bytes → 421 replies, no crash.
- `PRIVMSG #x :` (empty trailing) → 412.

---

## DAY 12 — CAP + client compatibility

**Learn**
- Why irssi sends `CAP LS 302` first (read `Server::cmdCap`). HexChat/weechat if available.

**Test**
- irssi, HexChat, weechat all connect and complete registration without errors.
- Double PASS/USER → 462. NICK before PASS → allowed; JOIN before registration → 451.

---

## DAY 13 — Code review of your files

**Build**
- Re-read all 4 of your files top to bottom. Remove TODO comments. Check 42 norm-style:
  tabs, `_camelCase` members, no raw `new` leaks.
- Confirm every `Reply::err*` you send has the right numeric + text.

**Test**
- `make fclean && make re` from zero errors. Valgrind/leaks a session of join/part/kick.

---

## DAY 14 — Full dress rehearsal with yel-arib

**Test**
- You evaluate him, he evaluates you, using the official evaluation sheet, every box.
- 3 irssi clients + nc: full scenario: register, join, invite, mode, topic, kick, part, quit.
- Simulate a crash check: kill the server with clients connected, restart — clean state.

---

## DAY 15 — EVALUATION READY

**Build**
- Final `make re`, clean repo, `git tag final`, README links correct.
- Rehearse the 2-minute pitch: how poll() loop works, how you aggregate partial data,
  how a JOIN travels through the code (this exact question is asked every time).
- Make sure you can answer: "what happens if a client disconnects mid-PRIVMSG broadcast?"

**Defense cheat-sheet (must be able to explain)**
- `poll()` vs `select()`; why non-blocking; why SIGPIPE is ignored.
- Your read-buffer aggregation and the write queue.
- Operator precedence in MODE flags (yours: yel-arib's part, but know it).
- What `reinterpret_cast<sockaddr*>` does in bind/accept.
