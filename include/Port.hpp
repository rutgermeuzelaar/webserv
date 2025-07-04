#ifndef PORT_HPP
# define PORT_HPP
# include <inttypes.h>
# include <string>

class Port
{
    public:
        const uint16_t m_port;
        uint16_t from_string(const std::string&) const;
        const std::string to_string(void) const;
        Port(const std::string&);
        Port(uint16_t);
};

bool operator==(const Port&, const Port&);
#endif