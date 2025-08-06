#pragma once
#ifndef HTTP_HPP
# define HTTP_HPP
# include "Pch.hpp"
# include <string>
# include <optional>
# include <unordered_map>
# include <stdexcept>
# include <filesystem>
# include "PartialWriter.hpp"

class MultiPartChunk;

enum class HTTPMethod {
	GET,
	POST,
	DELETE,
	UNSUPPORTED
};

HTTPMethod StringToMethod(const std::string& method);
std::ostream& operator<<(std::ostream& os, HTTPMethod http_method);
std::string stringify(HTTPMethod);

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status
enum class HTTPStatusCode: int
{
    Continue = 100,
    SwitchingProtocols = 101,
    Processing = 102,
    EarlyHints = 103,
    OK = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,
    MultiStatus = 207,
    AlreadyReported = 208,
    IMUsed = 226,
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    TemporaryRedirect = 307,
    PermanentRedirect = 308,
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    ContentTooLarge = 413,
    URITooLong = 414,
    UnsupportedMediaType = 415,
    RangeNotSatisfiable = 416,
    ExpectationFailed = 417,
    Imateapot = 418,
    MisdirectedRequest = 421,
    UnprocessableContent = 422,
    Locked = 423,
    FailedDependency = 424,
    TooEarly = 425,
    UpgradeRequired = 426,
    PreconditionRequired = 428,
    TooManyRequests = 429,
    RequestHeaderFieldsTooLarge = 431,
    UnavailableForLegalReasons = 451,
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HTTPVersionNotSupported = 505,
    VariantAlsoNegotiates = 506,
    InsufficientStorage = 507,
    LoopDetected = 508,
    NotExtended = 510,
    NetworkAuthenticationRequired = 511,
};

const char *get_http_status_text(HTTPStatusCode status);
bool is_http_status_code(int);
std::ostream& operator<<(std::ostream&, const HTTPStatusCode&);
HTTPStatusCode from_string(const std::string& str);


//-----------------------------------------------------------------------------
// Classes to parse HTTP requests
//-----------------------------------------------------------------------------
class HttpRequestStartLine
{
    private:
        HTTPMethod  m_http_method;
        std::string m_uri;
        std::string m_version;
        bool        m_complete;

        const std::string& parse_uri(const std::string& uri);
    public:
        HttpRequestStartLine(void);
        ~HttpRequestStartLine(void);
        HttpRequestStartLine& operator=(const HttpRequestStartLine&) = default;
        HttpRequestStartLine(std::string);

        HTTPMethod get_http_method(void) const;
        const std::string& get_uri(void) const;
        const std::string& get_version(void) const;
        bool complete(void) const;
};

class HttpHeaders
{
    private:
        size_t m_line_break_count;
        std::unordered_map<std::string, std::string> m_headers;

    public:
        HttpHeaders(void);
        HttpHeaders(const HttpHeaders&);
        HttpHeaders& operator=(const HttpHeaders&) = default;
        ~HttpHeaders(void);

        /**
         * @brief retrieves the value of a specific HTTP header
         * @param key name of the header to retrieve
         * @note made headers lowcase for handling case insensitivity
         * @return The value of the requested header, or empty string if not found
         */
        const std::string get_header(const std::string& header_name) const;
        const std::unordered_map<std::string, std::string>& get_headers() const;

        void set_headers(const std::unordered_map<std::string, std::string>& headers);
        void add_header(const std::string& raw_string);
        void add_header(const std::string& key, const std::string& value);
        bool complete(void) const;
};

std::tuple<std::string, std::unordered_map<std::string, std::string>> parse_content_type(const std::string&);
std::tuple<std::string, std::unordered_map<std::string, std::string>> parse_content_disposition(const std::string&);
std::optional<std::string> get_header_value(const std::string& header_str, const std::string& header_attr_name);

class ChunkedDecoder
{
private:
	enum State {
		READING_SIZE,
		READING_DATA,
		READING_TRAILER,
		COMPLETE
	};

	State m_state;
	size_t m_expected_chunk_size;
	size_t m_current_chunk_read;
	std::string m_buffer;
	std::string m_decoded;
	bool m_complete;

	bool parse_chunk_size(size_t& pos);
	bool parse_chunk_data(size_t& pos);
	bool parse_trailer(size_t& pos);

public:
	ChunkedDecoder();
	void append(const std::string& data);
	bool complete() const;
	size_t hex_to_size(const std::string& hex_str);

	const std::string& get_decoded() const;
};

class MultiPartChunk
{
    private:
        std::string m_mime_type;
        std::tuple<std::string, std::unordered_map<std::string, std::string>> m_content_disposition;
    public:
        MultiPartChunk(void);
        MultiPartChunk(const MultiPartChunk&);
        MultiPartChunk& operator=(const MultiPartChunk&) = default;
        ~MultiPartChunk(void);
        std::string m_data;
        HttpHeaders m_headers;
        void parse_header_attributes(void);
        const std::string& get_mime_type(void) const;
};

class HttpBody: public PartialWriter
{
    private:
        std::string m_raw;
        size_t      m_index;
        size_t      m_data_start;
        std::optional<std::string> m_boundary;
        ulong m_content_length;
        size_t m_client_max_body_size;
        bool m_initialized;
        bool m_complete;
        MultiPartChunk m_chunk;
        const HttpHeaders* m_headers;
		bool m_is_chunked;
        std::string m_content_type;
		ChunkedDecoder m_chunked_decoder;

        void parse(void);
    public:
        HttpBody(void);
        HttpBody(const HttpHeaders* headers, HTTPMethod method, size_t client_max_body_size);
        HttpBody(const HttpBody&);
        HttpBody& operator=(const HttpBody&) = default;

        bool complete(void) const;
        bool initialized(void) const;
        void append(const std::string&);
        bool is_chunked(void) const;

        const std::string& get_raw(void) const;
        const MultiPartChunk& get_multi_part_chunk(void) const;
        const ChunkedDecoder& get_chunked_decoder(void) const;
};

class Request
{
    private:
        HttpRequestStartLine _start_line; //* status line components
        HttpHeaders _headers; //* key-value headers
        HttpBody _body;
        std::string _raw;
        size_t _index; // index of current byte
        size_t _line_count; // count parsed_lines

        void parse(void);
    public:
        Request();
        ~Request(); 
        Request& operator=(const Request&) = default;

        void append(const char* buffer, size_t len, size_t client_max_body_size);
        bool complete(void) const;
		bool is_empty() const;

        //* getters
        HttpRequestStartLine& getStartLine();
        HttpHeaders& getHeaders();
        HttpBody& getBody();
        const HttpBody& getBody() const;

        const HttpRequestStartLine& getStartLine() const;
        const HttpHeaders& getHeaders() const;

        void printRequest() const;
};

//-----------------------------------------------------------------------------
// Classes to create HTTP responses
//-----------------------------------------------------------------------------
class HTTPStatusLine
{
    private:
        static std::string  m_protocol;
        std::string         m_status_text;           
        HTTPStatusCode      m_status_code;

    public:
        HTTPStatusLine(HTTPStatusCode status_code);
		HTTPStatusLine(const HTTPStatusLine& src);
        HTTPStatusLine& operator=(const HTTPStatusLine& other);
        
        //* getters
        const std::string& getStatusText() const { return m_status_text; }
        HTTPStatusCode getStatusCode() const { return m_status_code; }
        const std::string& getProtocol() const { return m_protocol; }
};

std::ostream& operator<<(std::ostringstream&, const HTTPStatusLine& http_status_line);

class Response: public PartialWriter
{
private:
    HTTPStatusLine                      _status_line;
    HttpHeaders                         _headers; //* key-value headers
    std::string                         _body;
    int                                 _client_fd;
    bool                                _headers_complete;
    size_t                              _headers_size; // Byte count of status line, headers and linebreak
    size_t                              _body_size;

    void headersToBytes(void);
public:
    Response(HTTPStatusCode status_code = HTTPStatusCode::OK);
	Response(const Response& src);
    Response& operator=(const Response& src);
    Response(Response&& other) noexcept;
    ~Response();

    //* setters
    void setHeader(const std::string& key, const std::string& value);
    void setContentType(const std::string& type, const std::string& charset = "UTF-8");
    void setDate();
    void setLastModified(const std::string& date);
    void setCacheControl(const std::string& value);
    void setServer(const std::string& server = "webserv/1.0");

    void setBody(const std::string& content);
    void setBodyFromFile(const std::filesystem::path& file_path);
    void redirect(const std::string& location, HTTPStatusCode status_code = HTTPStatusCode::Found);
    void setClientFD(int client_fd);
    void setStatusLine(HTTPStatusCode status_code);

    //* getters
    HTTPStatusCode getStatusCode() const;
    std::string getStatusText() const;
    std::string getHeader(const std::string& key) const;
    HttpHeaders& getHeaders(void);
    int getClientFD(void) const;
    size_t getHeadersSize(void) const;
    size_t getBodySize(void) const;
	bool getHeadersComplete(void) const;

    void printResponse() const; //* for debugging
	void finalize(void);
    void printHeaders() const;
    void printBody() const;
};
//-----------------------------------------------------------------------------
// Error handling when something bad happens, indicate error with HTTP status
// code
//-----------------------------------------------------------------------------
class HTTPException : public std::runtime_error {
private:
    HTTPStatusCode _status_code;

public:
    HTTPException(HTTPStatusCode status_code, const std::string& message = "");
    HTTPStatusCode getStatusCode() const;
};
#endif
