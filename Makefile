SOURCES_STANDARD := main.cpp HTTPRequestMethod.cpp HTTPResponse.cpp HTTPStatusCode.cpp \
HTTPStatusLine.cpp Request.cpp
NAME := webserv

run: re
	./$(NAME)

include common.mk