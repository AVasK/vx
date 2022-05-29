#include <iostream>
#include <variant>
#include <tuple>
#include <any>

#include "meta.hpp"

namespace vx {

// =====[ at ]=====
template <size_t I> struct at_index : std::in_place_index_t<I> {};
template <size_t I> inline constexpr at_index<I> at;


// template <typename... Ts, size_t I>
// decltype(auto) operator| (std::variant<Ts...> & var, at_index<I>) {
//     return std::get<I>(var);
// }

// template <typename... Ts, size_t I>
// decltype(auto) operator| (std::variant<Ts...> const& var, at_index<I>) {
//     return std::get<I>(var);
// }

// template <typename... Ts, size_t I>
// decltype(auto) operator| (std::variant<Ts...> && var, at_index<I>) {
//     return std::get<I>(var);
// }

// template <typename... Ts, size_t I>
// decltype(auto) operator| (std::tuple<Ts...> & var, at_index<I>) {
//     return std::get<I>(var);
// }

template <typename T, size_t I>
decltype(auto) operator| (T && v, at_index<I>) {
    return std::get<I>(std::forward<T>(v));
}

// =====[ as ]=====
template <typename T> struct convert_to : std::in_place_type_t<T> {};
template <typename T> inline constexpr convert_to<T> as;

// general case : as acts as a static_cast for non-variant types
template <typename From, typename To>
constexpr auto operator| (From const& value, convert_to<To>) {
    return static_cast<To>(value);
}

// variant |as <T>
template <typename... Ts, typename Type>
decltype(auto) operator| (std::variant<Ts...> & variant, convert_to<Type>) {
    return std::get<Type>(variant);
}

template <typename... Ts, typename Type>
decltype(auto) operator| (std::variant<Ts...> const& variant, convert_to<Type>) {
    return std::get<Type>(variant);
}

template <typename... Ts, typename Type>
decltype(auto) operator| (std::variant<Ts...> && variant, convert_to<Type>) {
    return std::get<Type>(std::move(variant));
}

// any |as <T>
template <typename Type>
decltype(auto) operator| (std::any & a, convert_to<Type>) {
    return std::any_cast<Type>(a);
}

template <typename Type>
decltype(auto) operator| (std::any const& a, convert_to<Type>) {
    return std::any_cast<Type>(a);
}

template <typename Type>
decltype(auto) operator| (std::any && a, convert_to<Type>) {
    return std::any_cast<Type>(std::move(a));
}


// =====[ is ]=====
template <typename T> struct compare {};
template <typename T> inline constexpr compare<T> is {};

// variant |is <T>
template <typename... Ts, typename Type>
// requires( meta::try_find<Type, Ts...>() |as <bool> )
bool  operator| (std::variant<Ts...> const& variant, compare<Type>) {
    return std::holds_alternative<Type>(variant);
}

// any |is <T>
template <typename Type>
bool  operator| (std::any const& a, compare<Type>) {
    return a.type() == typeid(Type);
}

}// namespace vx;
