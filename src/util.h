#ifndef CATA_UTIL_H
#define CATA_UTIL_H

#include <type_traits>
#include <memory>
#include <algorithm>

//--------------------------------------------------------------------------------------------------
// make_unique as defined by C++14.
//--------------------------------------------------------------------------------------------------
template <typename T, typename... Args>
inline typename std::enable_if<
    !std::is_array<T>::value,
    std::unique_ptr<T>>::type
make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
inline typename std::enable_if<
    std::is_array<T>::value && std::extent<T>::value == 0,
    std::unique_ptr<T>>::type
make_unique(size_t const size) {
    using elem_t = typename std::remove_extent<T>::type;
	return std::unique_ptr<T>(new elem_t[size]());
}

template <typename T, typename... Args>
typename std::enable_if<std::extent<T>::value != 0, void>::type
make_unique(Args&&...) = delete;

//==================================================================================================
// Common math or numeric helpers.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
//! get the sign of a number.
template <typename T>
inline constexpr int signum(T const x, std::false_type) {
    return T {0} < x;
}

template <typename T>
inline constexpr int signum(T const x, std::true_type) {
    return (T {0} < x) - (x < T {0});
}

template <typename T>
inline constexpr int signum(T const x) {
    return signum(x, std::is_signed<T>());
}

//--------------------------------------------------------------------------------------------------
//! clamp n to the closed interval [lo, hi].
template <typename T>
inline T clamp(T const n, T const lo, T const hi) {
    return n < lo ? lo : n > hi ? hi : n;
}

//--------------------------------------------------------------------------------------------------
//! clamp n to the half-open interval [lo, +inf).
template <typename T>
inline T clamp_min(T const n, T const lo) {
    return n < lo ? lo : n;
}

//--------------------------------------------------------------------------------------------------
//! clamp n to the half-open interval (-inf, hi].
template <typename T>
inline T clamp_max(T const n, T const hi) {
    return n > hi ? hi : n;
}

//--------------------------------------------------------------------------------------------------
//! "make bit flag"
template <typename T = int>
inline constexpr T mbf(unsigned const n) {
    return (T {1} << n);
}

//--------------------------------------------------------------------------------------------------
//! get the total number of elements in a (multi-dimensional) array.
//! i.e. array_elements(arr[2][3]) == 6
template <typename T>
inline constexpr typename std::enable_if<std::is_array<T>::value, int>::type
array_elements(T const&) {
    return sizeof(T) / sizeof(typename std::remove_all_extents<T>::type);
}

template <typename T>
inline constexpr typename std::enable_if<!std::is_array<T>::value, void>::type
array_elements(T const&) = delete;

//==================================================================================================
// Container-wide versions of some common stl algorithms.
// These try to be ADL friendly (hence the using statements) to support user-defined begin() end().
//==================================================================================================

//--------------------------------------------------------------------------------------------------
//! As std::find_if(begin, end, predicate)
//! @note The result type is not fully ADL friendly (Easily fixed under C++14).
template <typename Container, typename Predicate>
inline auto find_if(Container const &c, Predicate p) -> decltype(c.begin()) {
    using std::begin;
    using std::end;
    return std::find_if(begin(c), end(c), p);
}

//--------------------------------------------------------------------------------------------------
//! A fusion of std::find_if(begin, end, predicate) and std::map::find.
//! Return a pair [iterator, bool], where the bool indicates whether a value was found.
//! @note The result type is not fully ADL friendly (Easily fixed under C++14).
template <typename Container, typename Predicate>
inline auto locate_if(Container &c, Predicate p) -> std::pair<decltype(c.begin()), bool> {
    using std::begin;
    using std::end;
    auto const last = end(c);
    auto const it = std::find_if(begin(c), last, p);
    return std::make_pair(it, it != last);
}

//--------------------------------------------------------------------------------------------------
//! As std::any_of(begin, end, predicate)
template <typename Container, typename Predicate>
inline bool any_of(Container const &c, Predicate p) {
    using std::begin;
    using std::end;
    return std::any_of(begin(c), end(c), p);
}

//--------------------------------------------------------------------------------------------------
//! As std::none_of(begin, end, predicate)
template <typename Container, typename Predicate>
inline bool none_of(Container const &c, Predicate p) {
    using std::begin;
    using std::end;
    return std::none_of(begin(c), end(c), p);
}

//--------------------------------------------------------------------------------------------------
//! As std::copy_if(begin, end, it, predicate)
template <typename Container, typename OutputIterator, typename Predicate>
inline OutputIterator copy_if(Container const &c, OutputIterator it, Predicate p) {
    using std::begin;
    using std::end;
    return std::copy_if(begin(c), end(c), it, p);
}

//--------------------------------------------------------------------------------------------------
//! As std::transform(begin, end, it, unary_function)
template <typename Container, typename OutputIterator, typename UnaryFunction>
inline OutputIterator transform(Container const &c, OutputIterator it, UnaryFunction f) {
    using std::begin;
    using std::end;
    return std::transform(begin(c), end(c), it, f);
}

//--------------------------------------------------------------------------------------------------
//! The remove-erase idiom: Container::erase(std::remove_if(begin, end, predicate), end)
template <typename Container, typename Predicate>
inline void remove_erase_if(Container &c, Predicate p) {
    using std::begin;
    using std::end;
    
    auto const last = end(c);
    c.erase(std::remove_if(begin(c), last, p), last);
}

#endif // CATA_UTIL_H
