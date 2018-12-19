#ifndef SORBET_TYPECASE_H
#define SORBET_TYPECASE_H

#include "common/Exception.h"
#include "common/common.h"
#include <functional>
#include <string>
#include <typeinfo>

namespace sorbet {
// taken from https://stackoverflow.com/questions/22822836/type-switch-construct-in-c11
// should be replaced by variant when we're good with c++17

// Begin ecatmur's code
template <typename T> struct remove_class {};
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...)> { using type = R(A...); };
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) const> { using type = R(A...); };
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) volatile> { using type = R(A...); };
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) const volatile> {
    using type = R(A...);
};

template <typename T> struct get_signature_impl {
    using type = typename remove_class<decltype(&std::remove_reference<T>::type::operator())>::type;
};
template <typename R, typename... A> struct get_signature_impl<R(A...)> { using type = R(A...); };
template <typename R, typename... A> struct get_signature_impl<R (&)(A...)> { using type = R(A...); };
template <typename R, typename... A> struct get_signature_impl<R (*)(A...)> { using type = R(A...); };
template <typename T> using get_signature = typename get_signature_impl<T>::type;
// End ecatmur's code

// Begin typecase code

template <typename T> struct argtype_extractor : public argtype_extractor<decltype(&T::operator())> {};
template <typename ClassType, typename ReturnType, typename ArgType>
struct argtype_extractor<ReturnType (ClassType::*)(ArgType *) const>
// we specialize for pointers to member function
{
    using arg_type = ArgType;
};

template <typename Base, typename FUNC> bool typecaseHelper(Base *base, FUNC &&func) {
    typedef argtype_extractor<std::function<get_signature<FUNC>>> traits;
    typedef typename traits::arg_type ArgType;
    if (ArgType *first = fast_cast<Base, ArgType>(base)) {
        func(first);
        return true;
    } else {
        return false;
    }
}

template <typename Base, typename... Subclasses> void typecase(Base *base, Subclasses &&... funcs) {
    bool done = false;

    bool UNUSED(dummy[sizeof...(Subclasses)]) = {(done = done || typecaseHelper<Base>(base, funcs))...};

    if (!done) {
        if (!base) {
            sorbet::Exception::raise("nullptr passed to typecase");
        }
        sorbet::Exception::raise("not handled typecase case: ", demangle(typeid(*base).name()));
    }
}
} // namespace sorbet

// End typecase code
#endif // SORBET_TYPECASE_H
