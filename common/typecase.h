#ifndef SORBET_TYPECASE_H
#define SORBET_TYPECASE_H

#include "common/common.h"
#include "common/exception/Exception.h"
#include "common/has_member.h"
#include <functional>
#include <string>
#include <typeinfo>

namespace sorbet {
// taken from https://stackoverflow.com/questions/22822836/type-switch-construct-in-c11
// should be replaced by variant when we're good with c++17

// Begin ecatmur's code
template <typename T> struct remove_class {};
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...)> {
    using type = R(A...);
};
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) const> {
    using type = R(A...);
};
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) volatile> {
    using type = R(A...);
};
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) const volatile> {
    using type = R(A...);
};

template <typename T> struct get_signature_impl {
    using type = typename remove_class<decltype(&std::remove_reference<T>::type::operator())>::type;
};
template <typename R, typename... A> struct get_signature_impl<R(A...)> {
    using type = R(A...);
};
template <typename R, typename... A> struct get_signature_impl<R (&)(A...)> {
    using type = R(A...);
};
template <typename R, typename... A> struct get_signature_impl<R (*)(A...)> {
    using type = R(A...);
};
template <typename T> using get_signature = typename get_signature_impl<T>::type;
// End ecatmur's code

// Begin typecase code

template <typename T> struct argtype_extractor : public argtype_extractor<decltype(&T::operator())> {};
template <typename ClassType, typename ReturnType, typename ArgType>
struct argtype_extractor<ReturnType (ClassType::*)(ArgType) const> {
    using arg_type = ArgType;
};

// fast_cast-based typecase

template <typename Base, typename FUNC> bool typecaseHelper(Base *base, FUNC &&func) {
    using traits = argtype_extractor<std::function<get_signature<FUNC>>>;
    // we specialize for pointers to member function
    using ArgType = typename std::remove_pointer<typename traits::arg_type>::type;
    if (ArgType *first = fast_cast<Base, ArgType>(base)) {
        func(first);
        return true;
    } else {
        return false;
    }
}

GENERATE_HAS_MEMBER(tag)

template <typename Base, typename... Subclasses> void typecase(Base *base, Subclasses &&...funcs) {
    static_assert(HAS_MEMBER_tag<Base>() != true,
                  "For tagged pointers, please call typecase on a reference to the object not a pointer.");
    bool done = (false || ... || typecaseHelper<Base>(base, funcs));

    if (!done) {
        if (!base) {
            sorbet::Exception::raise("nullptr passed to typecase");
        }
        sorbet::Exception::raise("not handled typecase case: {}", demangle(typeid(*base).name()));
    }
}

// Tagged-pointer based typecase

template <typename Base, typename FUNC> bool typecaseHelper(Base &base, FUNC &&func) {
    using traits = argtype_extractor<std::function<get_signature<FUNC>>>;
    // We specialize (const and non-const) references
    using ArgType = typename std::remove_const<typename std::remove_reference<typename traits::arg_type>::type>::type;
    if (Base::template isa<ArgType>(base)) {
        func(Base::template cast<ArgType>(base));
        return true;
    } else {
        return false;
    }
}

template <typename Base, typename... Subclasses> void typecase(Base &base, Subclasses &&...funcs) {
    static_assert(HAS_MEMBER_tag<Base>() == true,
                  "typecase used on reference type without .tag()! Did you mean to use it on a pointer type?");
    bool done = (false || ... || typecaseHelper<Base>(base, funcs));

    if (!done) {
        if (!base) {
            sorbet::Exception::raise("nullptr passed to typecase");
        }
        sorbet::Exception::raise("not handled typecase case: {}", base.tag());
    }
}

} // namespace sorbet

// End typecase code
#endif // SORBET_TYPECASE_H
