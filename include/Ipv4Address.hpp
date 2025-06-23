#ifndef IPV4_ADDRESS_HPP
# define IPV4_ADDRESS_HPP
# include <string>
# include <inttypes.h>

class Ipv4Address
{
    public:
        uint32_t m_address;
        Ipv4Address(const std::string);
        Ipv4Address(int32_t);
        uint32_t from_string(const std::string&) const;
        std::string to_string(void) const;
};

std::vector<std::string> split(const std::string to_split, const char delimiter);
bool operator==(const Ipv4Address&, const Ipv4Address&);
#endif