#include "output_base.h"
#include "debug.h"

#include <cstring>
#include <cstdio>
#include <cerrno>
#include <algorithm> // find for truncate_string

#define CATA_TRUNCATE_AT_ETX 0

//--------------------------------------------------------------------------------------------------
namespace {
//--------------------------------------------------------------------------------------------------
// The original code truncated formatted strings at an embedded ETX('\003').
// The purpose is very unclear. The code is never used as far as I can tell from setting
// a breakpoint to fire on finding it.
//
// CATA_TRUNCATE_AT_ETX allows the behavior to be toggled until it can be understood what the
// the intent and purpose of the original code was.
//--------------------------------------------------------------------------------------------------
#if CATA_TRUNCATE_AT_ETX
size_t truncate_string(char *const buffer, size_t const size) noexcept
{
    auto const beg = buffer;
    auto const end = buffer + size;
    auto const it  = std::find(beg, end, '\003');
    
    if (it != end) {
        *it = '\0'; //found it; make it null
    }

    return static_cast<size_t>(it - beg);
}
#else
size_t truncate_string(char *const buffer, size_t const size) noexcept
{
    (void)buffer;
    return size;
}
#endif

//--------------------------------------------------------------------------------------------------
// Safe versions of strcpy
// TODO: move elsewhere.
//--------------------------------------------------------------------------------------------------
#if defined(_MSC_VER)
void string_copy(char *const dst, size_t const dst_size, char const *src) noexcept {
    strncpy_s(dst, dst_size, src, _TRUNCATE);
}
#else
void string_copy(char *const dst, size_t const dst_size, char const *src) noexcept {
    auto const src_size = strlen(src);
    strncpy(dst, src, std::min(dst_size, src_size));
}
#endif
//--------------------------------------------------------------------------------------------------
void set_error_string(char *const buffer, size_t const buffer_size,
    char const *const format, int const e_code)
{
    string_copy(buffer, buffer_size, "Error when formatting string.");

    DebugLog(D_WARNING, D_GAME) << "Error '" << strerror(e_code) <<
        "' when formatting string '" << format << "'.";
}

//--------------------------------------------------------------------------------------------------
//! A buffer via a std::string.
//--------------------------------------------------------------------------------------------------
struct sprintf_string_buffer {
    enum : size_t { buffer_size = 16 }; // size of the small string optimization on MSVC.

    size_t resize(size_t const new_size) {
        // String makes enough room for the null terminator itself.
        str.resize(new_size - 1, '\0');
        return size();
    }
    
    char* data() noexcept {
        return &str[0];
    }

    size_t size() const noexcept {
        return str.size() + 1; // String has room for a final null.
    }

    sprintf_string_buffer() {
        resize(buffer_size);
    }

    std::string str;
};

#if defined(_MSC_VER)
//--------------------------------------------------------------------------------------------------
// MSVC implementation.
//--------------------------------------------------------------------------------------------------
int try_format(char *const buffer, size_t const buffer_size, char const *const format, va_list args)
{
    // Get the required buffer size;
    int const result = _vscprintf_p(format, args);
    if (result == -1) {
        set_error_string(buffer, buffer_size, format, errno);
        return -1;
    }

    // Plus one more for the null
    int const required_size = result + 1;
    if (static_cast<size_t>(required_size) <= buffer_size) {
        _vsprintf_p(buffer, buffer_size, format, args);
    }

    return required_size;
}
#else
//--------------------------------------------------------------------------------------------------
// General implementation; no positional arguments on MSVC
//--------------------------------------------------------------------------------------------------
int try_format(char *const buffer, size_t const buffer_size, char const *const format, va_list args)
{
    // Clear errno before trying
    errno = 0;

    va_list args_copy;
    va_copy(args_copy, args);
    auto const result = vsnprintf(buffer, buffer_size, format, args_copy);
    va_end(args_copy);

    // Standard conformant versions return -1 on error only.
    // Some non-standard versions return -1 to indicate a bigger buffer is needed.
    if (result < 0) {
        // Was it actually an error?
        if (auto const e_code = errno) {
            set_error_string(buffer, buffer_size, format, e_code);
            return -1;
        }
        
        // No, we need a bigger buffer; try double what we have now.
        return buffer_size * 2;
    }
    
    // We know how much buffer was / will be used; don't forget the null.
    return result + 1;
}
#endif
//--------------------------------------------------------------------------------------------------
//! The actual implementation of that works with a generic Buffer object, and deligates to a
//! platform specific implementation.
//--------------------------------------------------------------------------------------------------
template <typename Buffer>
Buffer try_format(Buffer buffer, char const *const format, va_list args)
{
    // Keep trying while the buffer is too small.
    for (;;) {
        size_t const cur_size = buffer.size();
        int    const result   = try_format(buffer.data(), cur_size, format, args);

        if (result == -1 || buffer.resize(static_cast<size_t>(result)) <= cur_size) {
            break; //error, or big enough.
        }
    }

    truncate_string(buffer.data(), buffer.size());

    return buffer;
}
//--------------------------------------------------------------------------------------------------
} //namespace
//--------------------------------------------------------------------------------------------------
std::string string_format(char const *const format, ...)
{
    va_list ap;
    va_start(ap, format);
    auto result = vstring_format(format, ap);
    va_end(ap);

    return result;
}

//--------------------------------------------------------------------------------------------------
std::string vstring_format(char const *const format, va_list argptr)
{
    return try_format(sprintf_string_buffer {}, format, argptr).str;
}

//--------------------------------------------------------------------------------------------------
formatted_buffer buffer_format(char const *const format, ...)
{
    va_list ap;
    va_start(ap, format);
    auto result = vbuffer_format(format, ap);
    va_end(ap);

    return result;
}

//--------------------------------------------------------------------------------------------------
formatted_buffer vbuffer_format(char const *const format, va_list argptr)
{
    return try_format(formatted_buffer {}, format, argptr);
}
