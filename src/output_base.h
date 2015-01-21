#ifndef CATA_OUTPUT_BASE_H
#define CATA_OUTPUT_BASE_H

#include <array>
#include <string>
#include <cstdarg>

//--------------------------------------------------------------------------------------------------
//! A buffer optimised for use as a C-string.
//! First uses a stack allocated buffer, and once exhaused switches to a heap allocated buffer.
//! Implemented as a discriminated union; mind the boilerplate.
//--------------------------------------------------------------------------------------------------
struct formatted_buffer {
    enum : size_t { intial_buffer_size = 512 };

    using array_t = std::array<char, intial_buffer_size>;

    formatted_buffer() {
        new (&arr) array_t;
        arr[0] = '\0';
    }

    formatted_buffer(formatted_buffer &&other) noexcept
      : cur_size {other.cur_size}, is_dynamic {other.is_dynamic}
    {
        if (is_dynamic) {
            new (&str) std::string(std::move(other.str));
        } else {
            new (&arr) array_t(std::move(other.arr));
        }
    }

    formatted_buffer& operator=(formatted_buffer &&rhs) noexcept {
        this->~formatted_buffer();

        if (rhs.is_dynamic) {
            new (&str) std::string(std::move(rhs.str));
        } else {
            new (&arr) array_t(std::move(rhs.arr));
        }

        cur_size = rhs.cur_size;
        is_dynamic = rhs.is_dynamic;

        return *this;
    }

    ~formatted_buffer() {
        if (is_dynamic) {
            str.~basic_string();
        }
    }

    size_t resize(size_t const new_size) {
        if (is_dynamic) {
            str.resize(new_size - 1, '\0');             // string includes a null terminator (-1)
        } else if (new_size > intial_buffer_size) {
            new (&str) std::string(new_size - 1, '\0'); // string includes a null terminator (-1)
            is_dynamic = true;
        }

        return (cur_size = new_size);
    }

    char* data() noexcept {
        return !is_dynamic ? &arr[0] : &str[0];
    }

    char const* c_str() const noexcept {
        return !is_dynamic ? &arr[0] : &str[0];
    }

    std::string to_string() {
        return !is_dynamic ? std::string(&arr[0], cur_size) : std::move(str);
    }

    size_t size() const noexcept {
        return cur_size;
    }

    template <typename Stream>
    friend Stream& operator<<(Stream &lhs, formatted_buffer const &rhs) {
        lhs << rhs.c_str();
        return lhs;
    } 

    union {
        array_t     arr;
        std::string str;
    };

    size_t cur_size   = intial_buffer_size;
    bool   is_dynamic = false;
};

//--------------------------------------------------------------------------------------------------
//! printf style formatted output to a std::string. Use this in the general case where a real
//! string is needed (i.e to store in a variable for later).
//--------------------------------------------------------------------------------------------------
std::string string_format(char const *pattern, ...);
std::string vstring_format(char const *pattern, va_list argptr);

//--------------------------------------------------------------------------------------------------
//! printf style formatted output to a buffer object. Use this when an actual instance of a string
//! isn't required (i.e when passing a char const* to another function).
//--------------------------------------------------------------------------------------------------
formatted_buffer buffer_format(char const *pattern, ...);
formatted_buffer vbuffer_format(char const *pattern, va_list argptr);

#endif // CATA_OUTPUT_BASE_H
