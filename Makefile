CC := g++
INC_DIRS := include
INC := $(foreach dir, $(INC_DIRS), -I$(dir))
CFLAGS := -Wall -Wextra -Werror -Wshadow $(INC) -g -std=c++20
LDFLAGS := -fsanitize=address
OBJDIR := build
VPATH = $(shell find src/ -maxdepth 1 -mindepth 1)
SOURCES_STANDARD := Config.cpp ConfigDirective.cpp HttpContext.cpp Ipv4Address.cpp Lexer.cpp \
LocationContext.cpp Parser.cpp Port.cpp Scanner.cpp ServerContext.cpp HTTPException.cpp \
HTTPStatusCode.cpp main.cpp Request.cpp HTTPStatusLine.cpp Response.cpp Socket.cpp \
Utilities.cpp RequestHandler.cpp Server.cpp Client.cpp Epoll.cpp HttpHeaders.cpp HttpBody.cpp \
HttpMethod.cpp HttpRequestStartLine.cpp MultiPartChunk.cpp
NAME := webserv
OBJECTS_STANDARD := $(SOURCES_STANDARD:%.cpp=$(OBJDIR)/%.o)
OBJECTS_SHARED := $(SOURCES_SHARED:%.cpp=$(OBJDIR)/%.o)
OBJECTS_BONUS := $(SOURCES_BONUS:%.cpp=$(OBJDIR)/%.o)

.PHONY: all

all: $(NAME)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJECTS_STANDARD) $(OBJECTS_SHARED)
	$(CC) $(LDFLAGS) $(OBJECTS_STANDARD) $(OBJECTS_SHARED) -o $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJECTS_STANDARD) $(OBJECTS_SHARED) $(OBJECTS_BONUS)

fclean: clean
	rm -rf $(NAME)

re: fclean all
