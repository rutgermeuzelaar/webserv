#pragma once
#ifndef SESSION_HPP
# define SESSION_HPP
# include <string>
# include <unordered_map>
# include <optional>
# include "HttpMethod.hpp"

class Session
{
	private:
		const std::string m_id;
		std::unordered_map<std::string, std::string> m_data;
        std::unordered_map<HTTPMethod, size_t> m_request_history;
	public:
		Session(const std::string& id);
        Session& operator=(const Session&) = delete;
        Session(const Session&);
        ~Session();

		const std::string& get_id(void) const;
		void set_data_pair(const std::string& key, const std::string& value);
		std::optional<const std::string> get_value(const std::string& key) const;
        const std::unordered_map<HTTPMethod, size_t>& get_request_history() const;
        void add_request(HTTPMethod http_method);
        void print(void) const;
};
#endif