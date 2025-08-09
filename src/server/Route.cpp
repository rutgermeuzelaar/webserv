#include "Pch.hpp"
#include <optional>
#include "Server.hpp"
#include "Utilities.hpp"
#include "Defines.hpp"

Route::Route(const std::string& url, const std::vector<HTTPMethod>& allowed_methods, route_act_t action)
    : m_url {url}
    , m_allowed_methods {allowed_methods}
    , m_action {action}
{

}

Route::Route(const Route& other)
    : m_url {other.m_url}
    , m_allowed_methods {other.m_allowed_methods}
    , m_action {other.m_action}
{

}

const std::string& Route::get_url(void) const
{
    return m_url;
}

const std::vector<HTTPMethod>& Route::get_allowed_methods(void) const
{
    return m_allowed_methods;
}

route_act_t Route::get_action(void) const
{
    return m_action;
}

static const std::string session_created_page(const std::string& name, const std::string& favorite_color)
{
    return (
        "<!DOCTYPE html>"
        "<html lang=\"en-US\">"
        "<head>"
		    "<meta charset=\"utf-8\"/>"
            "<link rel=\"stylesheet\" href=\"/root/css/stylesheet.css\">"
		    "<title>Session created</title>"
        "</head>"
        "<body>"
            "<h1 style=\"color:" + favorite_color + "\">Welcome " + name + "</h1>"
            "<p>We will now start tracking you.</p>"
            "<p><a href=\"/\">Home</a></p>"
        "</body>"
        "</html"
    );
}

Response create_session(Server& server, Request& request, const LocationContext* location, const ServerContext& config)
{
    Response response(HTTPStatusCode::OK);
    std::optional<std::string> session_opt = get_header_value(request.getHeaders().get_header("Cookie"), "session");
    const std::string& post_body = request.getBody().get_raw();
    auto attributes = parse_query_string(post_body);
    SessionHandler& session_handler = server.getSessionHandler();
    std::string session_id;

    if (!has_keys(attributes, {"name", "favorite-color"}))
    {
        return build_error_page(HTTPStatusCode::BadRequest, location, config);
    }
     if (session_opt.has_value())
    {
        session_id = session_opt.value();
        auto session_it = session_handler.find_session(session_id);
        // session already exists, redirect
        if (session_it != session_handler.get_sessions().end())
        {
            Return return_obj(HTTPStatusCode::SeeOther, "/");
            return build_redirect(return_obj);
        }
    }
    session_id = session_handler.add_session();
    response.setHeader("Set-Cookie", session_handler.get_cookie(session_id));
    response.setHeader("Content-Length", "0");
    response.setBody(session_created_page(attributes.at("name"), attributes.at("favorite-color")));
    auto session_it = session_handler.find_session(session_id);
    session_it->second.set_data_pair("name", attributes.at("name"));
    session_it->second.set_data_pair("favorite-color", attributes.at("favorite-color"));
    response.print();
    return response;
}

static std::string file_upload_success_page(void)
{
    return (
        "<!DOCTYPE html>"
        "<html lang=\"en-US\">"
            "<head>"
                "<meta charset=\"utf-8\"/>"
                "<link rel=\"stylesheet\" href=\"/root/css/stylesheet.css\">"
                "<title>Upload successful</title>"
            "</head>"
            "<body>"
                "<h1>upload succesful</h1>"
                "<p>"
                    "Your file was uploaded succesfully. You can view uploaded files <a href=\"/cgi-bin/delete-entry.py/upload\">here</a>."
                "</p>"
                "<p><a href=\"/\">home</a></p>"
            "</body>"
        "</html>"
    );
};

Response file_upload_success(Server& server, Request& request, const LocationContext* location, const ServerContext& config)
{
    Response response(HTTPStatusCode::OK);

    (void)server, (void)request, (void)location, (void)config;
    response.setBody(file_upload_success_page());
    return (response);
}

static std::string whoami_default_page(void)
{
    return (
        "<!DOCTYPE html>"
        "<html lang=\"en-US\">"
            "<head>"
                "<meta charset=\"utf-8\"/>"
                "<link rel=\"stylesheet\" href=\"/root/css/stylesheet.css\">"
                "<title>whoami</title>"
            "</head>"
            "<body>"
                "<h1>whoami</h1>"
                "<p>"
                    "I don't know you. Click <a href=\"/register.html\">here</a> so I can get to know you."
                "</p>"
                "<p><a href=\"/\">home</a></p>"
            "</body>"
        "</html>"
    );
}

static std::string whoami_page(const Session& session)
{
    const std::string name = session.get_value("name").value();
    const std::string favorite_color = session.get_value("favorite-color").value();

    std::string request_stats_html = "<b>Your http request stats</b><ul>";

    const auto& request_history = session.get_request_history();

    for (const auto& it: request_history)
    {
        request_stats_html.append(
            "<li>" + 
            stringify(it.first) +
            ": " +
            std::to_string(it.second) +
            " times</li>"
        );
    }
    request_stats_html.append("</ul>");
    return (
        "<!DOCTYPE html>"
        "<html lang=\"en-US\">"
            "<head>"
                "<meta charset=\"utf-8\"/>"
                "<link rel=\"stylesheet\" href=\"/root/css/stylesheet.css\">"
                "<title>whoami</title>"
            "</head>"
            "<body>"
                "<h1>whoami</h1>"
                "<blockquote cite=\"https://www.youtube.com/watch?v=sNPnbI1arSE\">"
                "<p><i>"
                    "Hi, my name is, what? My name is, who?<br>"
                    "My name is, chka-chka, <s>Slim Shady</s>"
                "</i></p>"
                "</blockquote>"
                "<p>"
                "<b>Your are</b><br>" +
                name + "<br><br>" +
                "<b>This is your favorite color</b><br>" +
                "<span style=\"color:" + favorite_color + "\">" + favorite_color + "</span><br><br>" +
                request_stats_html +
                "<p><a href=\"/\">home</a></p>"
            "</body>"
        "</html>"
    );
};

Response whoami(Server& server, Request& request, const LocationContext* location, const ServerContext& config)
{
    Response response(HTTPStatusCode::OK);
    std::optional<std::string> session_opt = get_header_value(request.getHeaders().get_header("Cookie"), "session");
    SessionHandler& session_handler = server.getSessionHandler();
    std::string session_id;

    (void)location, (void)config;
    if (session_opt.has_value())
    {
        session_id = session_opt.value();
        auto session_it = session_handler.find_session(session_id);
        // session already exists, redirect
        if (session_it != session_handler.get_sessions().end())
        {
            response.setBody(whoami_page(session_it->second));
            return response;
        }
    }
    response.setBody(whoami_default_page());
    return response;
}
