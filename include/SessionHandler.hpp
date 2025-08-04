#pragma once
#ifndef SESSION_HANDLER_HPP
# define SESSION_HANDLER_HPP
# include <unordered_map>
# include <string>
# include "Session.hpp"

class SessionHandler
{
    private:
        std::unordered_map<std::string, Session> m_sessions;
		const std::string generate_session_id(void);

    public:
        std::unordered_map<std::string, Session>::iterator find_session(const std::string& id);
        std::unordered_map<std::string, Session>::const_iterator find_session_const(const std::string& id) const;
        void clear_session(const std::string& id);
        const std::string add_session(void);
        const std::string get_cookie(const std::string& id) const;
        bool has_session(const std::string& id) const;
        const std::unordered_map<std::string, Session>& get_sessions(void) const;
};
#endif