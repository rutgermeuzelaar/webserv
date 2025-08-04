#include "Session.hpp"
#include <cstdlib>
#include <algorithm>
#include "HttpMethod.hpp"

Session::Session(const std::string& id)
	: m_id {id}
{

}

Session::Session(const Session& other)
    : m_id {other.m_id}
    , m_data {other.m_data}
    , m_request_history {other.m_request_history}
{

}

Session::~Session()
{

}

const std::string& Session::get_id() const
{
    return m_id;
}

void Session::set_data_pair(const std::string& key, const std::string& value)
{
    m_data[key] = value;
}

std::optional<const std::string> Session::get_value(const std::string& key) const
{
    const auto& value = std::find_if(
        m_data.begin(),
        m_data.end(),
        [&key](const std::pair<std::string, std::string>& pair) {
            return pair.first == key;
        }
    );
    if (value != m_data.end())
    {
        return value->second;
    }
    return std::nullopt;
}

void Session::add_request(HTTPMethod http_method)
{
    m_request_history[http_method]++;
}

void Session::print(void) const
{
    std::cout << "Printing data for session id: " << m_id << '\n';
    std::cout << "m_data\n"; 
    for (const auto& it: m_data)
    {
        std::cout << it.first << ": " << it.second << '\n';
    }
    std::cout << "m_request_history\n";
    for (const auto& it: m_request_history)
    {
        std::cout << it.first << ": " << it.second << '\n';
    }
}

const std::unordered_map<HTTPMethod, size_t>& Session::get_request_history() const
{
    return m_request_history;
}
