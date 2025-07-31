#include "Session.hpp"
#include <cstdlib>

const std::string Session::generate_session_id()
{
	static const char* hex = "0123456789abcdef";

	std::string session_id;
	session_id.reserve(32);
	for (size_t i = 0; i < 32; ++i)
	{
		session_id[i] = hex[std::rand() % 16];
	}
	return session_id;
}

Session::Session()
	: m_id {generate_session_id()}
{

}
