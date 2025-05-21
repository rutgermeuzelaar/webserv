SOURCES_STANDARD := parseTests.cpp HTTPStatusCode.cpp HTTPStatusLine.cpp Request.cpp Response.cpp
NAME := webserv

run: $(NAME)
	./$(NAME)

include common.mk