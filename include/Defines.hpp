#ifndef DEFINES_HPP
# define DEFINES_HPP
#  define PROTOCOL "HTTP/1.1"
#  define LINE_BREAK "\r\n" // https://www.rfc-editor.org/rfc/rfc2616#section-2.2
#  define WHITE_SPACE " \f\n\r\t\v"
#  define MAX_HEADER_SIZE 8192
#  define RECV_BUFFER_SIZE 4096
#  define MAX_URI_SIZE 2048
#  define DEFAULT_CONF "./root/default.conf"
#  define CGI_DIR "cgi-bin"
#  define CGI_TIMEOUT_MS 450
#  define STYLESHEET "/root/css/stylesheet.css"
#endif