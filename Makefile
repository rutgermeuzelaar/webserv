SOURCES_STANDARD := main.cpp Request.cpp
NAME := webserv

run: re
	./$(NAME)

include common.mk