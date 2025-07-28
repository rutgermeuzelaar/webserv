#pragma once
#ifndef PARTIAL_WRITER_HPP
# define PARTIAL_WRITTER_HPP
# include <vector>
# include <cstddef>
# include <string>

class PartialWriter
{
    private:
        std::vector<std::byte>  m_bytes;
        size_t                  m_bytes_sent;

    public:
        PartialWriter();
        PartialWriter(size_t bytes_size);
        PartialWriter& operator=(const PartialWriter&) = default;
        PartialWriter(const PartialWriter&) = default;
        ~PartialWriter() = default;

        void set_bytes(const std::vector<std::byte>& bytes);

        std::vector<std::byte>& get_bytes(void);
        const std::vector<std::byte>& get_bytes(void) const;

        void append_bytes(const std::vector<std::byte>& bytes);
        void append_bytes(const std::string& bytes);

        const std::byte* get_next_bytes(size_t* length);
        void increment_bytes_sent(size_t amount);
        bool fully_sent(void) const;
        void print(void) const;
};
#endif