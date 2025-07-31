#include "Pch.hpp"
#include "PartialWriter.hpp"
#include "Utilities.hpp"

PartialWriter::PartialWriter()
    : m_bytes_sent {0}
{

}

PartialWriter::PartialWriter(size_t bytes_size)
    : m_bytes_sent {0}
{
    m_bytes.reserve(bytes_size);
}

void PartialWriter::set_bytes(const std::vector<std::byte>& bytes)
{
    m_bytes = bytes;
}

void PartialWriter::append_bytes(const std::vector<std::byte>& bytes)
{
    m_bytes.insert(
        m_bytes.end(),
        bytes.begin(),
        bytes.end()
    );
}

void PartialWriter::append_bytes(const std::string& bytes)
{
    copy_str_bytes(m_bytes, bytes);
}

const std::byte* PartialWriter::get_next_bytes(size_t *length)
{
    const size_t bytes_length = m_bytes.size() - m_bytes_sent;

    *length = bytes_length;
    return (m_bytes.data() + m_bytes_sent);
}

void PartialWriter::increment_bytes_sent(size_t amount)
{
    m_bytes_sent += amount;
}

bool PartialWriter::fully_sent(void) const
{
    return m_bytes_sent == m_bytes.size();
}

std::vector<std::byte>& PartialWriter::get_bytes()
{
    return m_bytes;
}

const std::vector<std::byte>& PartialWriter::get_bytes() const
{
    return m_bytes;
}

void PartialWriter::print() const
{
    for (size_t i = 0; i < m_bytes.size(); ++i)
    {
        std::putchar(static_cast<char>(m_bytes[i]));
    }
}

void PartialWriter::print(size_t start, size_t end) const
{
    for (size_t i = start; i < end; ++i)
    {
        std::putchar(static_cast<char>(m_bytes[i]));
    }
}
