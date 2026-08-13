# ft_irc — 15-day plan (2 people)

Reference client: **irssi**. Test everything with irssi AND `nc -C`.

> Already done (setup): poll loop, non-blocking sockets, SIGINT/SIGPIPE handling,
> read-buffer aggregation, write queue, command dispatcher, numerics in `Reply`,
> PASS / NICK / USER / QUIT / PING / PONG / CAP, README, Makefile.
> Remaining work = the 7 commands marked `TODO` + polish.

---

## Week 1 — Core

| Day | yatanagh (channels / messaging) | yel-arib (modes / ops) |
|-----|----------------------------------|------------------------|
| 1 | `JOIN` (Join.cpp): create/join, errors 451/403/473/475/471, first member = op, topic+names replies | `MODE` view (Mode.cpp): 324 channel mode display |
| 2 | `JOIN` broadcast + multi-channel lists (`JOIN #a,#b k1,k2`), 353/366 names | `MODE +o/-o` with broadcast, 482/441 errors |
| 3 | `PRIVMSG`/`NOTICE` (PrivMsg.cpp): channel + nick targets, 411/412/401/404 | `MODE +i/-i` and `+t/-t` with broadcast |
| 4 | `PART` (Part.cpp) + QUIT broadcast to all shared channels (already in core — verify) | `MODE +k/-k` and `+l/-l`, 472 unknown mode |
| 5 | **Milestone 1**: 2 irssi users chat in a channel, see joins/parts | **Milestone 1**: ops work, modes reflect in JOIN checks |

## Week 2 — Operator commands + integration

| Day | yatanagh | yel-arib |
|-----|----------|----------|
| 6 | `KICK` (Kick.cpp): 461/403/482/441 + broadcast | `INVITE` (Invite.cpp): 461/442/401/443, 341 reply |
| 7 | Invite-aware JOIN (use `Client::addInvite` / `isInvitedTo`) | `TOPIC` (Topic.cpp): view 332/331, set with +t check 482 |
| 8 | Empty-channel cleanup after KICK/PART | MODE arg parsing edge cases (`+kl 10 key` ordering) |
| 9 | Buffer stress: 10+ clients, slow TCP, long messages | Nick validation, ERR_ALREADYREGISTRED double-PASS/USER |
| 10 | **Milestone 2**: full eval-sheet scenario green for channels | **Milestone 2**: all 5 mode flags green in irssi |

## Week 3 — Hardening + defense

| Day | yatanagh | yel-arib |
|-----|----------|----------|
| 11 | Partial-data tests (`nc -C`, Ctrl+D), multiple commands per packet | SIGINT clean shutdown, valgrind/leaks on macOS |
| 12 | CAP negotiation with irssi/HexChat, stray `\r` / `\n` handling | Error-path audit: every command's missing-arg + unknown cases |
| 13 | Fix 512-byte line limit, truncation, overlong channel/topic edge cases | README finalize (resources + AI section), Makefile relink check |
| 14 | Full pair test session: irssi + nc, every command, 3 people | Same + take notes of any crash/edge case, fix together |
| 15 | **Final**: `make re`, clean repo, git tag, mock defense with eval sheet | Same — evaluate each other using the official evaluation sheet |

---

## Command ownership (files)

| File | Owner | Contents |
|------|-------|----------|
| `srcs/commands/Join.cpp` | yatanagh | JOIN (multi, keys, errors) |
| `srcs/commands/PrivMsg.cpp` | yatanagh | PRIVMSG + NOTICE |
| `srcs/commands/Part.cpp` | yatanagh | PART |
| `srcs/commands/Kick.cpp` | yatanagh | KICK |
| `srcs/commands/Mode.cpp` | yel-arib | MODE (i, t, k, o, l) |
| `srcs/commands/Invite.cpp` | yel-arib | INVITE |
| `srcs/commands/Topic.cpp` | yel-arib | TOPIC |
| shared: `includes/*.hpp`, `srcs/Server.cpp`, `srcs/Reply.cpp` | both | agree before touching |

## Ground rules

- One branch per person (`yatanagh`, `yel-arib`); merge into `main` daily.
- Never push a commit that doesn't compile (`make re` before every push).
- If you touch a shared file, tell the other person in the same session.
- Test with the reference client every day, not just `nc`.

## Numeric replies cheat-sheet (already in `Reply.hpp`)

401, 403, 404, 411, 412, 421, 431, 432, 433, 441, 442, 443, 451, 461, 462, 464,
471, 472, 473, 475, 476, 482, 501, 502 / 221, 324, 331, 332, 333, 341, 353, 366, 372, 376

Use `Reply::err*` / `Reply::rpl*` helpers — they format `:server CODE nick ...` correctly.
