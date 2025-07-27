CXX := g++
INC_DIRS := include
INC := $(foreach dir, $(INC_DIRS), -I$(dir))
CXXFLAGS := -Wall -Wextra -Werror -Wshadow -Wvla $(INC) -std=c++17 -DNDEBUG
LDFLAGS :=
OBJDIR := build
VPATH = $(shell find src/ -maxdepth 1 -mindepth 1)
SOURCES_STANDARD := Config.cpp ConfigDirective.cpp HttpContext.cpp Ipv4Address.cpp Lexer.cpp \
LocationContext.cpp Parser.cpp Port.cpp Scanner.cpp ServerContext.cpp HTTPException.cpp \
HTTPStatusCode.cpp main.cpp Request.cpp HTTPStatusLine.cpp Response.cpp Socket.cpp \
Utilities.cpp RequestHandler.cpp Server.cpp Client.cpp Epoll.cpp HttpHeaders.cpp HttpBody.cpp \
HttpMethod.cpp HttpRequestStartLine.cpp MultiPartChunk.cpp Cgi.cpp CgiProcess.cpp \
ResponseHandler.cpp
NAME := webserv
OBJECTS_STANDARD := $(SOURCES_STANDARD:%.cpp=$(OBJDIR)/%.o)
OBJECTS_SHARED := $(SOURCES_SHARED:%.cpp=$(OBJDIR)/%.o)
OBJECTS_BONUS := $(SOURCES_BONUS:%.cpp=$(OBJDIR)/%.o)
PCH := include/Pch.hpp.gch

.PHONY: all

all: $(NAME)

debug: CXXFLAGS += -g -pg -fsanitize=address
debug: LDFLAGS += -pg -fsanitize=address
debug: CXXFLAGS := $(filter-out -DNDEBUG,$(CXXFLAGS))
debug: $(NAME)

$(PCH): include/Pch.hpp
	$(CXX) $(CXXFLAGS) -c -o $(PCH) include/Pch.hpp

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(NAME): $(PCH) $(OBJECTS_STANDARD) $(OBJECTS_SHARED)
	$(CXX) $(LDFLAGS) $(OBJECTS_STANDARD) $(OBJECTS_SHARED) -o $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJECTS_STANDARD) $(OBJECTS_SHARED) $(OBJECTS_BONUS)

fclean: clean
	rm -rf $(NAME)

re: fclean all
