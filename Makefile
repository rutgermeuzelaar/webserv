CC := g++
INC_DIRS := include
INC := $(foreach dir, $(INC_DIRS), -I$(dir))
CFLAGS := -Wall -Wextra -Werror -Wshadow $(INC) -g -std=c++17
LDFLAGS :=
OBJDIR := build
VPATH = $(shell find src/ -maxdepth 1 -mindepth 1)
SOURCES_STANDARD := Config.cpp ConfigDirective.cpp HttpContext.cpp Ipv4Address.cpp Lexer.cpp \
LocationContext.cpp Parser.cpp Port.cpp Scanner.cpp ServerContext.cpp HTTPException.cpp \
HTTPStatusCode.cpp main.cpp Request.cpp HTTPStatusLine.cpp Response.cpp socket.cpp \
Utilities.cpp RequestHandler.cpp
NAME := webserv
OBJECTS_STANDARD := $(SOURCES_STANDARD:%.cpp=$(OBJDIR)/%.o)
OBJECTS_SHARED := $(SOURCES_SHARED:%.cpp=$(OBJDIR)/%.o)
OBJECTS_BONUS := $(SOURCES_BONUS:%.cpp=$(OBJDIR)/%.o)

.PHONY: all

all: $(NAME)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJECTS_STANDARD) $(OBJECTS_SHARED)
	$(CC) $(OBJECTS_STANDARD) $(OBJECTS_SHARED) -o $(NAME)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf $(OBJECTS_STANDARD) $(OBJECTS_SHARED) $(OBJECTS_BONUS)

fclean: clean
	rm -rf $(NAME)

re: fclean all
