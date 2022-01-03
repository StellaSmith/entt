#ifndef ENTT_CORE_HASHED_STRING_HPP
#define ENTT_CORE_HASHED_STRING_HPP

#include <cstddef>
#include <cstdint>
#include "../config/config.h"
#include "fwd.hpp"

namespace entt {

/**
 * @cond TURN_OFF_DOXYGEN
 * Internal details not to be documented.
 */

namespace internal {

template<typename>
struct fnv1a_traits;

template<>
struct fnv1a_traits<std::uint32_t> {
    using type = std::uint32_t;
    static constexpr std::uint32_t offset = 2166136261;
    static constexpr std::uint32_t prime = 16777619;
};

template<>
struct fnv1a_traits<std::uint64_t> {
    using type = std::uint64_t;
    static constexpr std::uint64_t offset = 14695981039346656037ull;
    static constexpr std::uint64_t prime = 1099511628211ull;
};

} // namespace internal

/**
 * Internal details not to be documented.
 * @endcond
 */

/**
 * @brief Zero overhead unique identifier.
 *
 * A hashed string is a compile-time tool that allows users to use
 * human-readable identifers in the codebase while using their numeric
 * counterparts at runtime.<br/>
 * Because of that, a hashed string can also be used in constant expressions if
 * required.
 *
 * @warning
 * This class doesn't take ownership of user-supplied strings nor does it make a
 * copy of them.
 *
 * @tparam Char Character type.
 */
template<typename Char>
class basic_hashed_string {
    using hs_traits = internal::fnv1a_traits<id_type>;

    struct const_wrapper {
        // non-explicit constructor on purpose
        constexpr const_wrapper(const Char *curr) ENTT_NOEXCEPT
            : str{curr} {}
        const Char *str;
    };

    struct helper_result {
        id_type hash;
        std::size_t size;
    };

    // Fowler–Noll–Vo hash function v. 1a - the good
    [[nodiscard]] static constexpr helper_result helper(const Char *curr) ENTT_NOEXCEPT {
        auto result = helper_result{hs_traits::offset, 0};

        while(*curr != 0) {
            result.hash = (result.hash ^ static_cast<hs_traits::type>(*(curr++))) * hs_traits::prime;
            ++result.size;
        }

        return result;
    }

    // Fowler–Noll–Vo hash function v. 1a - the good
    [[nodiscard]] static constexpr helper_result helper(Char const *curr, std::size_t size) ENTT_NOEXCEPT {
        auto result = helper_result{hs_traits::offset, size};

        while(size-- > 0) {
            result.hash = (result.hash ^ static_cast<hs_traits::type>(*(curr++))) * hs_traits::prime;
        }

        return result;
    }

public:
    /*! @brief Character type. */
    using value_type = Char;
    /*! @brief Unsigned integer type. */
    using hash_type = id_type;

    /**
     * @brief Returns directly the numeric representation of a string view.
     * @param str Human-readable identifer.
     * @param size Length of the string to hash.
     * @return The numeric representation of the string.
     */
    [[nodiscard]] static constexpr hash_type value(const value_type *str, std::size_t size) ENTT_NOEXCEPT {
        return helper(str, size).hash;
    }

    /**
     * @brief Returns directly the numeric representation of a string.
     *
     * Forcing template resolution avoids implicit conversions. An
     * human-readable identifier can be anything but a plain, old bunch of
     * characters.<br/>
     * Example of use:
     * @code{.cpp}
     * const auto value = basic_hashed_string<char>::value("my.png");
     * @endcode
     *
     * @tparam N Number of characters of the identifier.
     * @param str Human-readable identifer.
     * @return The numeric representation of the string.
     */
    template<std::size_t N>
    [[nodiscard]] static constexpr hash_type value(const value_type (&str)[N]) ENTT_NOEXCEPT {
        /* NOTE: `str` is expected to be an string literal, 
           and the size of it (`N`) contains then null terminator,
           which we dont hash. */
        return helper(str, N - 1).hash;
    }

    /**
     * @brief Returns directly the numeric representation of a string.
     * @param wrapper Helps achieving the purpose by relying on overloading.
     * @return The numeric representation of the string.
     */
    [[nodiscard]] static hash_type value(const_wrapper wrapper) ENTT_NOEXCEPT {
        return helper(wrapper.str).hash;
    }

    /*! @brief Constructs an empty hashed string. */
    constexpr basic_hashed_string() ENTT_NOEXCEPT = default;

private:
    explicit constexpr basic_hashed_string(value_type const *const ptr, helper_result const result) ENTT_NOEXCEPT
        : str{ptr},
          hash{result.hash},
          len{result.size} {}

public:
    /**
     * @brief Constructs a hashed string from a string given it's size.
     *
     * Example of use:
     * @code{.cpp}
     * entt::hashed_string{"my.png", 2} == "my"_hs;
     * @endcode
     *
     * @param ptr Human-readable identifer.
     * @param size The size of the string.
     */
    constexpr basic_hashed_string(value_type const *const ptr, std::size_t const size) ENTT_NOEXCEPT
        : basic_hashed_string(ptr, helper(ptr, size)) {}

    /**
     * @brief Constructs a hashed string from an array of const characters.
     *
     * Forcing template resolution avoids implicit conversions. An
     * human-readable identifier can be anything but a plain, old bunch of
     * characters.<br/>
     * Example of use:
     * @code{.cpp}
     * basic_hashed_string<char> hs{"my.png"};
     * @endcode
     *
     * @tparam N Number of characters of the identifier.
     * @param curr Human-readable identifer.
     */
    template<std::size_t N>
    constexpr basic_hashed_string(const value_type (&curr)[N]) ENTT_NOEXCEPT
        : basic_hashed_string(&curr[0], helper(&curr[0], N - 1)) {}

    /**
     * @brief Explicit constructor on purpose to avoid constructing a hashed
     * string directly from a `const value_type *`.
     *
     * @warning
     * The lifetime of the string is not extended nor is it copied.
     *
     * @param wrapper Helps achieving the purpose by relying on overloading.
     */
    explicit constexpr basic_hashed_string(const_wrapper wrapper) ENTT_NOEXCEPT
        : basic_hashed_string{wrapper.str, helper(wrapper.str)} {}

    /**
     * @brief Returns the human-readable representation of a hashed string.
     * @return The string used to initialize the instance.
     */
    [[nodiscard]] constexpr const value_type *data() const ENTT_NOEXCEPT {
        return str;
    }

    /**
     * @brief Returns the numeric representation of a hashed string.
     * @return The numeric representation of the instance.
     */
    [[nodiscard]] constexpr hash_type value() const ENTT_NOEXCEPT {
        return hash;
    }

    /*! @copydoc data */
    [[nodiscard]] constexpr operator const value_type *() const ENTT_NOEXCEPT {
        return data();
    }

    /**
     * @brief Returns the numeric representation of a hashed string.
     * @return The numeric representation of the instance.
     */
    [[nodiscard]] constexpr operator hash_type() const ENTT_NOEXCEPT {
        return value();
    }

    /**
     * @brief Returns the size a hashed string.
     * @return The size of the instance.
     */
    [[nodiscard]] constexpr std::size_t size() const ENTT_NOEXCEPT {
        return len;
    }

private:
    const value_type *str = {};
    hash_type hash{};
    std::size_t len{};
};

/**
 * @brief Deduction guide.
 *
 * It allows to deduce the character type of the hashed string directly from a
 * human-readable identifer provided to the constructor.
 *
 * @tparam Char Character type.
 * @tparam N Number of characters of the identifier.
 * @param str Human-readable identifer.
 */
template<typename Char, std::size_t N>
basic_hashed_string(const Char (&str)[N]) -> basic_hashed_string<Char>;

/**
 * @brief Compares two hashed strings.
 * @tparam Char Character type.
 * @param lhs A valid hashed string.
 * @param rhs A valid hashed string.
 * @return True if the two hashed strings are identical, false otherwise.
 */
template<typename Char>
[[nodiscard]] constexpr bool operator==(const basic_hashed_string<Char> &lhs, const basic_hashed_string<Char> &rhs) ENTT_NOEXCEPT {
    return lhs.value() == rhs.value();
}

/**
 * @brief Compares two hashed strings.
 * @tparam Char Character type.
 * @param lhs A valid hashed string.
 * @param rhs A valid hashed string.
 * @return True if the two hashed strings differ, false otherwise.
 */
template<typename Char>
[[nodiscard]] constexpr bool operator!=(const basic_hashed_string<Char> &lhs, const basic_hashed_string<Char> &rhs) ENTT_NOEXCEPT {
    return !(lhs == rhs);
}

/**
 * @brief Compares two hashed strings.
 * @tparam Char Character type.
 * @param lhs A valid hashed string.
 * @param rhs A valid hashed string.
 * @return True if the first element is less than the second, false otherwise.
 */
template<typename Char>
[[nodiscard]] constexpr bool operator<(const basic_hashed_string<Char> &lhs, const basic_hashed_string<Char> &rhs) ENTT_NOEXCEPT {
    return lhs.value() < rhs.value();
}

/**
 * @brief Compares two hashed strings.
 * @tparam Char Character type.
 * @param lhs A valid hashed string.
 * @param rhs A valid hashed string.
 * @return True if the first element is less than or equal to the second, false
 * otherwise.
 */
template<typename Char>
[[nodiscard]] constexpr bool operator<=(const basic_hashed_string<Char> &lhs, const basic_hashed_string<Char> &rhs) ENTT_NOEXCEPT {
    return !(rhs < lhs);
}

/**
 * @brief Compares two hashed strings.
 * @tparam Char Character type.
 * @param lhs A valid hashed string.
 * @param rhs A valid hashed string.
 * @return True if the first element is greater than the second, false
 * otherwise.
 */
template<typename Char>
[[nodiscard]] constexpr bool operator>(const basic_hashed_string<Char> &lhs, const basic_hashed_string<Char> &rhs) ENTT_NOEXCEPT {
    return rhs < lhs;
}

/**
 * @brief Compares two hashed strings.
 * @tparam Char Character type.
 * @param lhs A valid hashed string.
 * @param rhs A valid hashed string.
 * @return True if the first element is greater than or equal to the second,
 * false otherwise.
 */
template<typename Char>
[[nodiscard]] constexpr bool operator>=(const basic_hashed_string<Char> &lhs, const basic_hashed_string<Char> &rhs) ENTT_NOEXCEPT {
    return !(lhs < rhs);
}

/*! @brief Aliases for common character types. */
using hashed_string = basic_hashed_string<char>;

/*! @brief Aliases for common character types. */
using hashed_wstring = basic_hashed_string<wchar_t>;

inline namespace literals {

/**
 * @brief User defined literal for hashed strings.
 * @param str The literal without its suffix.
 * @return A properly initialized hashed string.
 */
[[nodiscard]] constexpr entt::hashed_string operator"" _hs(const char *str, std::size_t size) ENTT_NOEXCEPT {
    return entt::hashed_string{str, size};
}

/**
 * @brief User defined literal for hashed strings.
 * @param str The literal without its suffix.
 * @return A properly initialized hashed string.
 */
[[nodiscard]] constexpr entt::hashed_wstring operator"" _hs(const wchar_t *str, std::size_t size) ENTT_NOEXCEPT {
    return entt::hashed_wstring{str, size};
}

/**
 * @brief User defined literal for hashed wstrings.
 * @param str The literal without its suffix.
 * @return A properly initialized hashed wstring.
 */
[[nodiscard]] constexpr entt::hashed_wstring operator"" _hws(const wchar_t *str, std::size_t size) ENTT_NOEXCEPT {
    return entt::hashed_wstring{str, size};
}

} // namespace literals

} // namespace entt

#endif
