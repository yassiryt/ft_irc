NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -g

INCLUDES	= -Iincludes

SRCS		= srcs/main.cpp \
			  srcs/Server.cpp \
			  srcs/Client.cpp \
			  srcs/Channel.cpp \
			  srcs/Reply.cpp \
			  srcs/commands/Join.cpp \
			  srcs/commands/Part.cpp \
			  srcs/commands/PrivMsg.cpp \
			  srcs/commands/Kick.cpp \
			  srcs/commands/Invite.cpp \
			  srcs/commands/Topic.cpp \
			  srcs/commands/Mode.cpp

OBJS		= $(SRCS:.cpp=.o)
DEPS		= $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
