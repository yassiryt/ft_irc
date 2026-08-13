# yel-arib — Day-by-day guide to evaluation (MODE / INVITE / TOPIC)

Your scope: channel modes and operator actions. Your files:
`srcs/commands/Mode.cpp`, `Invite.cpp`, `Topic.cpp` (+ shared files, agreed with yatanagh).

Every day: `make re` before pushing, test with BOTH `nc -C` and irssi.
Start the server: `./ircserv 6667 pass`. Open irssi: `irssi -c 127.0.0.1 -p 6667 -w pass`.
irssi commands: `/mode #x +i`, `/invite nick #x`, `/topic #x hello`.

---

## DAY 1 — Learn modes + implement MODE view

**Learn**
- RFC 2812 §3.2.3 (MODE) and §3.1.3 (channel modes). Your flags: `i t k o l`.
- Re-read `Channel.hpp` — all mode getters/setters already exist (`isInviteOnly()`,
  `setKey()`, `setUserLimit()`...). You write the *logic and broadcasts* only.
- Study `Server::dispatch` and one finished command (`cmdNick`) to copy the style.

**Build** (`Mode.cpp`)
1. `args.size() < 2` (only `MODE <chan>`) → view mode: send `RPL_CHANNELMODEIS`:
   `:server 324 nick <chan> <+itkol> <args>` using `ch->getModeString()` / `getModeArgs()`.
2. `MODE` with no args at all → `ERR_NEEDMOREPARAMS`.
3. Channel null → `ERR_NOSUCHCHANNEL`.

**Test**
- `nc -C`: join #x, `MODE #x` → `324 nick #x +` (empty modes).
- `MODE #x +i` → `324` after → `+i` shows. irssi shows mode in channel header.

---

## DAY 2 — MODE +o / -o

**Learn**
- Operator lifecycle: first joiner gets op; empty channel loses ops (channel is deleted).
- `ch->isOperator()`, `addOperator()`, `removeOperator()`.

**Build**
1. Parse the mode string char by char: `+` = add, `-` = remove, letters are flags.
2. Before anything: `!ch->isOperator(&client)` → `ERR_CHANOPRIVSNEEDED`.
3. Flag `o` consumes one arg (a nick): find client (`getClientByNick`) → null → `ERR_NOSUCHNICK`;
   not member → `ERR_USERNOTINCHANNEL`.
4. `+o` → `addOperator`, `-o` → `removeOperator`.
5. Broadcast each change separately: `:<prefix> MODE <chan> +o <nick>`.

**Test**
- irssi: two users, op gives op (`/mode #x +o nick2`), then nick2 can use modes.
- `-o` on non-op → 482 (sender must be op). `+o ghost` → 401. `+o` non-member → 441.

---

## DAY 3 — MODE +i / +t

**Build**
1. Flags `i` and `t`: no args, call `setInviteOnly(bool)` / `setTopicRestricted(bool)`.
2. Broadcast `:prefix MODE <chan> +i` to all members.
3. Track the sign per flag (not per loop — `+i` then `-i` must both work).

**Test**
- `+i`: yatanagh's JOIN must reject you with 473 (Day 7 he wires invites; until then
  verify the 473 fires). `-i`: join works again.
- `+t`: non-op TOPIC set fails 482 (test again after yel-arib's Day 7 topic).

---

## DAY 4 — MODE +k / +l

**Build**
1. Flag `k` consumes one arg (the key): `+k` → `setKey(arg)`, `-k` → `clearKey()`.
   Broadcast includes the key for `+k`, nothing for `-k`.
2. Flag `l` consumes one arg: parse with `std::atoi`, reject non-numeric →
   ignore or `ERR_UNKNOWNMODE`; `+l` → `setUserLimit(n)`, `-l` → `clearUserLimit()`.
3. Unknown flag → `ERR_UNKNOWNMODE` for that flag, keep going.

**Test**
- `+k secret`: fresh join needs key (yatanagh's 475 path). `-k`: joins without key.
- `+l 2`: third user gets 471. `-l`: full channel joinable again.
- `MODE #x +z` → 472. `MODE #x +kl secret 5` → BOTH applied (order matters!).

---

## DAY 5 — MILESTONE 1: pair session

**Test with yatanagh**
1. All 5 flags: set in irssi, verify in nc, unset, verify.
2. Your modes must block his joins correctly (+i 473, +k 475, +l 471) — this is the
   cross-check that makes or breaks the eval.
3. Write the checklist: every flag, both signs, all error numerics.

---

## DAY 6 — INVITE

**Learn**
- RFC 2812 §3.2.7 (INVITE). The 4 checks + who gets what reply (341 to inviter,
  raw INVITE message to invited client).

**Build** (`Invite.cpp`)
1. `args.size() < 2` → `ERR_NEEDMOREPARAMS`; inviter not on channel → `ERR_NOTONCHANNEL`.
2. `ch->isInviteOnly()` and inviter not op → `ERR_CHANOPRIVSNEEDED`.
3. Target null → `ERR_NOSUCHNICK`; target already member → `ERR_USERONCHANNEL`.
4. Record invite: `target->addInvite(chanName)` AND `ch->addInvite(target)` (both sides —
   yatanagh's JOIN checks both).
5. `sendTo(inviter, RPL_INVITING 341: "<chan> <nick>")`.
6. `sendTo(*target, ":<inviter prefix> INVITE <target> :<chan>")`.

**Test**
- `+i` channel: op invites user → user sees INVITE, then JOIN succeeds.
- Non-op invites on +i channel → 482. Invite to member → 443. Invite from non-member → 442.

---

## DAY 7 — TOPIC

**Learn**
- RFC 2812 §3.2.4 (TOPIC). View vs set; `+t` restriction; empty topic clears.

**Build** (`Topic.cpp`)
1. `args.size() < 1` → `ERR_NEEDMOREPARAMS`; channel null → `ERR_NOSUCHCHANNEL`;
   sender not member → `ERR_NOTONCHANNEL`.
2. View mode (`args.size() == 1`): topic empty → `RPL_NOTOPIC` (331), else `RPL_TOPIC` (332).
3. Set mode: `ch->isTopicRestricted()` and not op → `ERR_CHANOPRIVSNEEDED`.
4. Empty trailing topic → `ch->clearTopic()` (still broadcast the change).
5. Broadcast `:<prefix> TOPIC <chan> :<topic>` to all members (including self);
   store `ch->setTopic(text, client.getNick())`.

**Test**
- irssi: `/topic #x` (view), `/topic #x hello` (set, everyone sees it).
- With `+t`: non-op set → 482; op set → works.
- TOPIC on #x you're not in → 442.

---

## DAY 8 — MODE edge cases

**Build**
1. Order handling: `+kl secret 5` sets both; `-l` needs no arg; missing arg for `+k`
   → just skip or 461 (pick one, be consistent — irssi sends `MODE #x +k` rarely).
2. Multiple nicks: `+o a b` style (`+oo a b`) — decide: support or 461. Most evals don't test it.
3. User MODE (`MODE nick ...`) → ignore silently (RFC allows, subject doesn't require).

**Test**
- `MODE #x +il 3` together. `MODE #x +i-i` same line. `MODE #x +++` → nothing, no crash.

---

## DAY 9 — Stress + cleanup audit

**Test**
- 10 clients, modes flip 50 times → all members see every broadcast, no crash.
- Mode set on channel you're not in → 482 first, not 324.
- Channel with only ops... `-o` last remaining op → channel has no ops but keeps existing
  members (correct; only first-joiner creates op).

---

## DAY 10 — MILESTONE 2: full scenario

**Test (with yatanagh)**
Official scenario: register 3, join, +i, invite, join, +t, topic, +k, join with key,
+k wrong key, +l, full, kick, part, quit. All on irssi. All green → move on.

---

## DAY 11 — Partial data + packet fuzzing on your commands

**Test**
- Split `MODE #x +i` into 5 pieces → applied once. 10 commands in one packet → all execute.
- `MODE #x +l abc` → no crash, no limit set. `TOPIC #x :a:b:c` → topic = "a:b:c"
  (trailing keeps colons). `INVITE` with 5 args → uses first two.

---

## DAY 12 — Client compatibility

**Test**
- irssi + HexChat + weechat: set modes, topic, invite from GUI → server behaves.
- irssi auto-sends `MODE` on join sometimes — don't crash on it.

---

## DAY 13 — Code review of your files

**Build**
- Re-read Mode.cpp/Invite.cpp/Topic.cpp top to bottom. Remove TODO comments.
- Verify every `Reply::` call uses the right numeric. Tabs, `_camelCase`, no leaks.

**Test**
- `make fclean && make re`. Valgrind a mode/invite/topic session.

---

## DAY 14 — Full dress rehearsal with yatanagh

**Test**
- You evaluate him, he evaluates you, official sheet, every box.
- Full scenario in 3 clients + nc, then SIGINT the server, restart, repeat — no stale state.

---

## DAY 15 — EVALUATION READY

**Build**
- Final `make re`, clean repo, `git tag final`.
- Rehearse explaining: MODE flag loop, why `+k` broadcasts the key, invite both-sides
  bookkeeping, and the whole mode-string parser.

**Defense cheat-sheet (must be able to explain)**
- `poll()` loop + why POLLOUT is set only when the write queue is non-empty.
- How you parse `+kl secret 5` (arg-consuming flags in order).
- Why empty channels are deleted and what that means for +t topic state.
- What happens if an operator leaves a channel with only regular users left (no ops — can
  anyone MODE? No. Is that RFC-correct? Yes, and say so).
