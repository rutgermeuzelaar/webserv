/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HTTPStatusCode.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: rmeuzela <rmeuzela@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/04/28 21:52:24 by rmeuzela      #+#    #+#                 */
/*   Updated: 2025/04/28 22:36:20 by rmeuzela      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPStatusCode.hpp"

const char *get_http_status_text(HTTPStatusCode status)
{
    switch (status)
    {
        case HTTPStatusCode::Continue: return "Continue";
        case HTTPStatusCode::SwitchingProtocols: return "Switching Protocols";
        case HTTPStatusCode::Processing: return "Processing";
        case HTTPStatusCode::EarlyHints: return "Early Hints";
        case HTTPStatusCode::OK: return "OK";
        case HTTPStatusCode::Created: return "Created";
        case HTTPStatusCode::Accepted: return "Accepted";
        case HTTPStatusCode::NonAuthoritativeInformation: return "Non-Authoritative Information";
        case HTTPStatusCode::NoContent: return "No Content";
        case HTTPStatusCode::ResetContent: return "Reset Content";
        case HTTPStatusCode::PartialContent: return "Partial Content";
        case HTTPStatusCode::MultiStatus: return "Multi-Status";
        case HTTPStatusCode::AlreadyReported: return "Already Reported";
        case HTTPStatusCode::IMUsed: return "IM Used";
        case HTTPStatusCode::MultipleChoices: return "Multiple Choices";
        case HTTPStatusCode::MovedPermanently: return "Moved Permanently";
        case HTTPStatusCode::Found: return "Found";
        case HTTPStatusCode::SeeOther: return "See Other";
        case HTTPStatusCode::NotModified: return "Not Modified";
        case HTTPStatusCode::TemporaryRedirect: return "Temporary Redirect";
        case HTTPStatusCode::PermanentRedirect: return "Permanent Redirect";
        case HTTPStatusCode::BadRequest: return "Bad Request";
        case HTTPStatusCode::Unauthorized: return "Unauthorized";
        case HTTPStatusCode::PaymentRequired: return "Payment Required";
        case HTTPStatusCode::Forbidden: return "Forbidden";
        case HTTPStatusCode::NotFound: return "Not Found";
        case HTTPStatusCode::MethodNotAllowed: return "Method Not Allowed";
        case HTTPStatusCode::NotAcceptable: return "Not Acceptable";
        case HTTPStatusCode::ProxyAuthenticationRequired: return "Proxy Authentication Required";
        case HTTPStatusCode::RequestTimeout: return "Request Timeout";
        case HTTPStatusCode::Conflict: return "Conflict";
        case HTTPStatusCode::Gone: return "Gone";
        case HTTPStatusCode::LengthRequired: return "Length Required";
        case HTTPStatusCode::PreconditionFailed: return "Precondition Failed";
        case HTTPStatusCode::ContentTooLarge: return "Content Too Large";
        case HTTPStatusCode::URITooLong: return "URI Too Long";
        case HTTPStatusCode::UnsupportedMediaType: return "Unsupported Media Type";
        case HTTPStatusCode::RangeNotSatisfiable: return "Range Not Satisfiable";
        case HTTPStatusCode::ExpectationFailed: return "Expectation Failed";
        case HTTPStatusCode::Imateapot: return "I'm a teapot";
        case HTTPStatusCode::MisdirectedRequest: return "Misdirected Request";
        case HTTPStatusCode::UnprocessableContent: return "Unprocessable Content";
        case HTTPStatusCode::Locked: return "Locked";
        case HTTPStatusCode::FailedDependency: return "Failed Dependency";
        case HTTPStatusCode::TooEarly: return "Too Early";
        case HTTPStatusCode::UpgradeRequired: return "Upgrade Required";
        case HTTPStatusCode::PreconditionRequired: return "Precondition Required";
        case HTTPStatusCode::TooManyRequests: return "Too Many Requests";
        case HTTPStatusCode::RequestHeaderFieldsTooLarge: return "Request Header Fields Too Large";
        case HTTPStatusCode::UnavailableForLegalReasons: return "Unavailable For Legal Reasons";
        case HTTPStatusCode::InternalServerError: return "Internal Server Error";
        case HTTPStatusCode::NotImplemented: return "Not Implemented";
        case HTTPStatusCode::BadGateway: return "Bad Gateway";
        case HTTPStatusCode::ServiceUnavailable: return "Service Unavailable";
        case HTTPStatusCode::GatewayTimeout: return "Gateway Timeout";
        case HTTPStatusCode::HTTPVersionNotSupported: return "HTTP Version Not Supported";
        case HTTPStatusCode::VariantAlsoNegotiates: return "Variant Also Negotiates";
        case HTTPStatusCode::InsufficientStorage: return "Insufficient Storage";
        case HTTPStatusCode::LoopDetected: return "Loop Detected";
        case HTTPStatusCode::NotExtended: return "Not Extended";
        case HTTPStatusCode::NetworkAuthenticationRequired: return "Network Authentication Required";
    }
    return nullptr;
}

std::ostream& operator<<(std::ostream& ostream, const HTTPStatusCode &http_status_code)
{
    ostream << static_cast<int>(http_status_code);
    return ostream;
}