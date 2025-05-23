SOURCES_STANDARD := main.cpp HTTPStatusCode.cpp \
HTTPStatusLine.cpp Request.cpp socket.cpp
NAME := webserv

run: re
	./$(NAME)

include common.mk