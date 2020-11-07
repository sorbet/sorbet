#ifndef SORBET_COMMON_HAS_METHOD_H
#define SORBET_COMMON_HAS_METHOD_H

#include <typeinfo>

namespace sorbet {

template <class> struct sfinae_true : std::true_type {};

/**
 * GENERATE_HAS_MEMBER(name, arg_types...) creates HAS_MEMBER_name<T>() which can be used to statically test if class
 * `T` contains a method `name` that accepts arguments `arg_types`.
 *
 * Example:
 *
 * GENERATE_HAS_MEMBER(toString, std::declval<const core::GlobalState &>())
 * static_assert(HAS_MEMBER_toString<core::NameRef>());
 * static_assert(!HAS_MEMBER_toString<std::string>());
 *
 * Adapted from https://stackoverflow.com/a/9154394
 */
#define GENERATE_HAS_MEMBER(name, arg_types...)                                                            \
    namespace __HAS_MEMBER_##name {                                                                        \
        template <class T>                                                                                 \
        static constexpr auto __has_##name(int)->sfinae_true<decltype(std::declval<T>().name(arg_types))>; \
        template <class> static constexpr auto __has_##name(long)->std::false_type;                        \
    };                                                                                                     \
    template <class T> constexpr bool HAS_MEMBER_##name() {                                                \
        return decltype(__HAS_MEMBER_##name::__has_##name<T>(0)){};                                        \
    }

} // namespace sorbet

#endif
