#include "Pch.hpp"
#include "Server.hpp"

std::unordered_map<std::string, Session>::iterator SessionHandler::find_session(const std::string& id)
{
    return m_sessions.find(id);
}

std::unordered_map<std::string, Session>::const_iterator SessionHandler::find_session_const(const std::string& id) const
{
    std::unordered_map<std::string, Session>::const_iterator it = m_sessions.find(id);

    return it;
}

void SessionHandler::clear_session(const std::string& id)
{
    auto it = find_session(id);

    if (it != m_sessions.end())
    {
        m_sessions.erase(it);
    }
}

const std::string SessionHandler::add_session()
{
    const std::string id = generate_session_id();

    m_sessions.insert({id, Session(id)});
    return id;
}

const std::string SessionHandler::generate_session_id()
{
	static const char* hex = "0123456789abcdef";

	std::string session_id;
	session_id.reserve(32);
	for (size_t i = 0; i < 32; ++i)
	{
		session_id.push_back(hex[std::rand() % 16]);
	}
	return session_id;
}

const std::string SessionHandler::get_cookie(const std::string& id) const
{
    return ("session=" + id + "; HttpOnly; SameSite=Strict");
}

bool SessionHandler::has_session(const std::string& id) const
{
    return (find_session_const(id) != m_sessions.end());
}

const std::unordered_map<std::string, Session>& SessionHandler::get_sessions(void) const
{
    return m_sessions;
}
