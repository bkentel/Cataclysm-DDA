#ifndef CATA_OUTPUT_BASE_H
#define CATA_OUTPUT_BASE_H

#include <array>
#include <vector>
#include <string>
#include <cstdarg>

//--------------------------------------------------------------------------------------------------
//! A buffer optimized for use as a C-string.
//! First uses a stack allocated buffer, and once exhaused switches to a heap allocated buffer.
//! Implemented as a discriminated union; mind the boilerplate.
//--------------------------------------------------------------------------------------------------
struct formatted_buffer {
    enum : size_t { intial_buffer_size = 512 };

    using array_t  = std::array<char, intial_buffer_size>;
    using vector_t = std::vector<char>;

    formatted_buffer() noexcept {
        new (&arr) array_t;
        arr[0] = '\0';
    }

    formatted_buffer(formatted_buffer &&other) noexcept
      : cur_size {other.cur_size}, is_static {other.is_static}
    {
        if (is_static) {
            new (&arr) array_t(std::move(other.arr));
        } else {
            new (&vec) vector_t(std::move(other.vec));
        }
    }

    formatted_buffer& operator=(formatted_buffer &&rhs) noexcept {
        this->~formatted_buffer();

        if (rhs.is_static) {
            new (&arr) array_t(std::move(rhs.arr));
        } else {
            new (&vec) vector_t(std::move(rhs.vec));
        }

        cur_size = rhs.cur_size;
        is_static = rhs.is_static;

        return *this;
    }

    ~formatted_buffer() {
        if (!is_static) {
            vec.~vector();
        }
    }

    size_t resize(size_t const new_size) {
        if (!is_static) {
            vec.resize(new_size, '\0');
        } else if (new_size > intial_buffer_size) {
            new (&vec) vector_t(new_size, '\0');
            is_static = false;
        }

        return (cur_size = new_size);
    }

    char* data() noexcept {
        return is_static ? &arr[0] : &vec[0];
    }

    char const* c_str() const noexcept {
        return is_static ? &arr[0] : &vec[0];
    }

    size_t size() const noexcept {
        return cur_size;
    }

    template <typename Stream>
    friend Stream& operator<<(Stream &lhs, formatted_buffer const &rhs) {
        lhs << rhs.c_str();
        return lhs;
    } 

#if defined(_MSC_VER)            // Momentarily disable a spurious warning
#   pragma warning(push)         // on MSVC 2015 about
#   pragma warning(disable:4624) // an implicitly deleted destructor in the union.
#endif
    union {
        array_t  arr;
        vector_t vec;
    };
#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

    size_t cur_size  = intial_buffer_size;
    bool   is_static = true;
};

//--------------------------------------------------------------------------------------------------
//! printf style formatted output to a std::string. Use this in the general case where a real
//! string is needed (i.e to store in a variable for later).
//--------------------------------------------------------------------------------------------------
std::string string_format(char const *format, ...);
std::string vstring_format(char const *format, va_list argptr);

//--------------------------------------------------------------------------------------------------
//! printf style formatted output to a buffer object. Use this when an actual instance of a string
//! isn't required (i.e when passing a char const* to another function).
//--------------------------------------------------------------------------------------------------
formatted_buffer buffer_format(char const *format, ...);
formatted_buffer vbuffer_format(char const *format, va_list argptr);

#endif // CATA_OUTPUT_BASE_H
