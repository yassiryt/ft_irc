*This project has been created as part of the 42 curriculum by yatanagh, yel-arib.*

# ft_irc

## Description

ft_irc is an IRC (Internet Relay Chat) server written in C++98, built as part of the 42 curriculum.
It implements the mandatory part of the subject:

- TCP/IP (v4) server using a **single `poll()`** loop and **non-blocking I/O** (no forking, no threads)
- Client registration with password authentication (`PASS`, `NICK`, `USER`)
- Channels with operator privileges
- Commands: `JOIN`, `PART`, `PRIVMSG`, `NOTICE`, `QUIT`, `PING`, `PONG`, `CAP` (negotiation)
- Operator commands: `KICK`, `INVITE`, `TOPIC`, `MODE` with flags `i`, `t`, `k`, `o`, `l`
- Handles partial data by aggregating received packets until a full command is available

## Instructions

### Build

```sh
make
```

### Run

```sh
./ircserv <port> <password>
```

### Test

Use any IRC client (reference client: **irssi**) or `nc`:

```sh
nc -C 127.0.0.1 <port>
```

Typical session:

```
PASS <password>
NICK mynick
USER myuser 0 * :My Real Name
JOIN #chan
PRIVMSG #chan :hello world
```

You can also send commands in several parts (Ctrl+D with `nc`) to test
packet aggregation:

```sh
$> nc -C 127.0.0.1 6667
com^Dman^Dd
```

## Resources

- [RFC 1459 - Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 - Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [RFC 2813 - Internet Relay Chat: Server Protocol](https://datatracker.ietf.org/doc/html/rfc2813)
- [Modern IRC Client Protocol (ircdocs)](https://modern.ircdocs.horse/)
- [The Beej Guide to Network Programming](https://beej.us/guide/bgnet/)
- `man poll`, `man socket`, `man fcntl`

### AI usage

AI was used during this project to:

- Design the initial class structure (`Server`, `Client`, `Channel`, `Reply`)
- Draft the Makefile, the poll-based main loop skeleton and the README
- Generate reference documentation summaries for RFC numeric replies

All AI-generated code was reviewed, tested and understood by both authors
before being integrated.
# ft_irc
