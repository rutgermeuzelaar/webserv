#pragma once
#ifndef SESSION_HPP
# define SESSION_HPP
# include <string>
# include <unordered_map>
# include <optional>

class Session
{
	private:
		const std::string m_id;
		std::unordered_map<std::string, std::string> m_data;
		
		const std::string generate_session_id(void);
	public:
		Session();

		const std::string& get_id(void) const;
		void set_data_pair(const std::string& key, const std::string& value);
		std::optional<const std::string> get_data(void) const;
};
#endif