#ifndef SRUBY_TYPECASE_H
#define SRUBY_TYPECASE_H

#include "Error.h"
#include "common.h"
#include <functional>
#include <string>
#include <typeinfo>

namespace ruby_typer {
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
template <typename Base, typename T> bool typecaseHelper(Base *base, std::function<void(T *)> func) {
    if (T *first = fast_cast<Base, T>(base)) {
        func(first);
        return true;
    } else {
        return false;
    }
}

template <typename Base, typename... Subclasses> void typecase(Base *base, Subclasses &&... funcs) {
    bool done = false;

    bool UNUSED(dummy[sizeof...(Subclasses)]) = {
        (done = done || typecaseHelper(base, std::function<get_signature<Subclasses>>(funcs)))...};

    if (!done) {
        ruby_typer::Error::check(false, "not handled case: ", demangle(typeid(base).name()));
    }
}
} // namespace ruby_typer

// End typecase code
#endif // SRUBY_TYPECASE_H
