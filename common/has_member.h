#ifndef SORBET_COMMON_HAS_MEMBER_H
#define SORBET_COMMON_HAS_MEMBER_H

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
#define GENERATE_HAS_MEMBER(name, arg_types...)                                                        \
    template <class T>                                                                                 \
    static constexpr auto __has_##name(int)->sfinae_true<decltype(std::declval<T>().name(arg_types))>; \
    template <class> static constexpr auto __has_##name(long)->std::false_type;                        \
    template <class T> static constexpr bool HAS_MEMBER_##name() { return decltype(__has_##name<T>(0)){}; }

/**
 * Given a method name, a default statement to run, and a list of argument types, do the following:
 * - Generate `HAS_MEMBER_method_name` to statically detect if method is defined on a class.
 * - Generate `CALL_MEMBER_method_name<T>` to call method `method_name` on object `T` if it exists otherwise return
 * `default_behavior`.
 *
 * Example:
 *
 * GENERATE_CALL_MEMBER(toString, return "", std::declval<const core::GlobalState &>())
 * CALL_MEMBER_toString<core::NameRef>::call(name, gs); // calls name.toString(gs)
 * CALL_MEMBER_toString<int>::call(10, gs); // returns ""
 */
#define GENERATE_CALL_MEMBER(method_name, default_behavior, arg_types...)                                   \
    GENERATE_HAS_MEMBER(method_name, arg_types)                                                             \
    template <typename T, bool has> class CALL_MEMBER_impl_##method_name {                                  \
    public:                                                                                                 \
        template <class... Args> static decltype(auto) call(T &self, Args &&...args) {                      \
            Exception::raise("should never be called");                                                     \
        };                                                                                                  \
    };                                                                                                      \
    template <typename T> class CALL_MEMBER_impl_##method_name<T, true> {                                   \
    public:                                                                                                 \
        template <class... Args> static decltype(auto) call(T &self, Args &&...args) {                      \
            return self.method_name(std::forward<Args>(args)...);                                           \
        };                                                                                                  \
    };                                                                                                      \
    template <typename T> class CALL_MEMBER_impl_##method_name<T, false> {                                  \
    public:                                                                                                 \
        template <class... Args> static decltype(auto) call(T &self, Args &&...args) { default_behavior; }; \
    };                                                                                                      \
    template <typename T>                                                                                   \
    class CALL_MEMBER_##method_name : public CALL_MEMBER_impl_##method_name<T, HAS_MEMBER_##method_name<T>()> {};

} // namespace sorbet

#endif
