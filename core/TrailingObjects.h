#ifndef SORBET_TRAILING_OBJECTS_H
#define SORBET_TRAILING_OBJECTS_H

#include "common/common.h"

// This file is largely copied from LLVM's llvm/Support/TrailingObjects.h, with some
// minor tweaks to bring in the very few things the original header needs from the
// rest of LLVM's infrastructure.  The original version was:
//
// https://github.com/llvm/llvm-project/blob/llvmorg-12.0.0/llvm/include/llvm/Support/TrailingObjects.h
//
// The license block from the original file:
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// A couple of notes on usage:
//
// 1. The TrailingObjects class only concerns itself with the computations for how
//    much space to allocate and how to access the trailing objects; it does not
//    perform any initialization of the objects themselves.  That is up to the
//    inheriting class.
//
// 2. As a corollary to that, the memory that you would need to access in the
//    constructor is completely uninitialized, so you will have to use either
//    placement `new` or functions like `std::uninitialized_{copy,move}{,_n}`
//    to initialize the trailing objects.
//
// 3. Similarly, any destruction of the trailing objects is entirely up to the
//    inheriting class; you will have to manually write out the calls to the
//    destructors.  (You cannot farm this out to TrailingObjects itself, because
//    at the point when `~TrailingObjects` would be called, the subclass object
//    technically does not exist anymore, so you cannot call methods -- e.g. the
//    methods to determine how many trailing objects there are -- on the subclass
//    object.)
//
// 4. Given that classes using TrailingObjects will want to call
//    `totalSizeToAlloc` to account for any trailing objects that need to be
//    allocated, and then call placement `new`, you will probably want to hide all
//    of that complexity behind a static factory function of some kind, and
//    therefore the constructor(s) of the class you are implementing should likely
//    be `private`.
//
// LLVM's documentation is reproduced below.

/// This header defines support for implementing classes that have
/// some trailing object (or arrays of objects) appended to them. The
/// main purpose is to make it obvious where this idiom is being used,
/// and to make the usage more idiomatic and more difficult to get
/// wrong.
///
/// The TrailingObject template abstracts away the reinterpret_cast,
/// pointer arithmetic, and size calculations used for the allocation
/// and access of appended arrays of objects, and takes care that they
/// are all allocated at their required alignment. Additionally, it
/// ensures that the base type is final -- deriving from a class that
/// expects data appended immediately after it is typically not safe.
///
/// Users are expected to derive from this template, and provide
/// numTrailingObjects implementations for each trailing type except
/// the last, e.g. like this sample:
///
/// \code
/// class VarLengthObj : private TrailingObjects<VarLengthObj, int, double> {
///   friend TrailingObjects;
///
///   unsigned NumInts, NumDoubles;
///   size_t numTrailingObjects(OverloadToken<int>) const { return NumInts; }
///  };
/// \endcode
///
/// You can access the appended arrays via 'getTrailingObjects', and
/// determine the size needed for allocation via
/// 'additionalSizeToAlloc' and 'totalSizeToAlloc'.
///
/// All the methods implemented by this class are are intended for use
/// by the implementation of the class, not as part of its interface
/// (thus, private inheritance is suggested).

namespace sorbet::core {

namespace internal {

template <typename First, typename... Rest> class AlignmentCalcHelper {
private:
    enum {
        FirstAlignment = alignof(First),
        RestAlignment = AlignmentCalcHelper<Rest...>::Alignment,
    };

public:
    enum { Alignment = FirstAlignment > RestAlignment ? FirstAlignment : RestAlignment };
};

template <typename First> class AlignmentCalcHelper<First> {
public:
    enum { Alignment = alignof(First) };
};

class TrailingObjectsBase {
protected:
    /// OverloadToken's purpose is to allow specifying function overloads
    /// for different types, without actually taking the types as
    /// parameters. (Necessary because member function templates cannot
    /// be specialized, so overloads must be used instead of
    /// specialization.)
    template <typename T> struct OverloadToken {};
};

template <int Align> class TrailingObjectsAligner : public TrailingObjectsBase {};
template <> class alignas(1) TrailingObjectsAligner<1> : public TrailingObjectsBase {};
template <> class alignas(2) TrailingObjectsAligner<2> : public TrailingObjectsBase {};
template <> class alignas(4) TrailingObjectsAligner<4> : public TrailingObjectsBase {};
template <> class alignas(8) TrailingObjectsAligner<8> : public TrailingObjectsBase {};
// LLVM includes 16 and 32, but Sorbet doesn't have anything that requires such alignment.

// Just a little helper for transforming a type pack into the same
// number of a different type. e.g.:
//   ExtractSecondType<Foo..., int>::type
template <typename Ty1, typename Ty2> struct ExtractSecondType {
    typedef Ty2 type;
};

// Compile-time log2.
template <size_t K> constexpr inline size_t CTLog2() {
    return 1 + CTLog2<K / 2>();
}

template <> constexpr inline size_t CTLog2<1>() {
    return 0;
}

constexpr inline bool isPowerOf2(uint64_t value) {
    return value && !(value & (value - 1));
}

inline unsigned log2_64(uint64_t value) {
    // std::countl_zero is in C++20; __builtin_clz is undefined at 0.
    uint64_t leading_zeros = value == 0 ? 64 : __builtin_clz(value);
    return 63 - leading_zeros;
}

// A stripped-down copy of llvm::Align to handle what we need below.
struct Align {
private:
    // log2 of the required alignment.
    uint8_t shiftValue = 0;

    explicit Align(uint64_t value) {
        shiftValue = value;
        ENFORCE(shiftValue < 64);
    }

public:
    constexpr Align() noexcept = default;
    constexpr Align(const Align &) = default;
    constexpr Align(Align &&) = default;
    Align &operator=(const Align &) = default;
    Align &operator=(Align &&) = default;

    uint64_t value() const {
        return uint64_t(1) << shiftValue;
    }

    template <typename T> constexpr static Align Of() {
        return Align(CTLog2<std::alignment_of_v<T>>());
    }
};

// This is a copy of `alignAddr` from llvm/Support/Alignment.h, but reduced to
// just the computation we need for this.
inline uintptr_t alignAddr(const void *addr, Align alignment) {
    uintptr_t p = reinterpret_cast<uintptr_t>(addr);
    uint64_t value = alignment.value();

    return (p + value - 1) & ~(value - uint64_t(1));
}

template <uint64_t Align> constexpr inline uint64_t alignTo(uint64_t value) {
    static_assert(Align != 0u);
    return (value + Align - 1) / Align * Align;
}

// TrailingObjectsImpl is somewhat complicated, because it is a
// recursively inheriting template, in order to handle the template
// varargs. Each level of inheritance picks off a single trailing type
// then recurses on the rest. The "Align", "BaseTy", and
// "TopTrailingObj" arguments are passed through unchanged through the
// recursion. "PrevTy" is, at each level, the type handled by the
// level right above it.

template <int Align, typename BaseTy, typename TopTrailingObj, typename PrevTy, typename... MoreTys>
class TrailingObjectsImpl {
    // The main template definition is never used -- the two
    // specializations cover all possibilities.
};

template <int Align, typename BaseTy, typename TopTrailingObj, typename PrevTy, typename NextTy, typename... MoreTys>
class TrailingObjectsImpl<Align, BaseTy, TopTrailingObj, PrevTy, NextTy, MoreTys...>
    : public TrailingObjectsImpl<Align, BaseTy, TopTrailingObj, NextTy, MoreTys...> {
    typedef TrailingObjectsImpl<Align, BaseTy, TopTrailingObj, NextTy, MoreTys...> ParentType;

    struct RequiresRealignment {
        static const bool value = alignof(PrevTy) < alignof(NextTy);
    };

    static constexpr bool requiresRealignment() {
        return RequiresRealignment::value;
    }

protected:
    // Ensure the inherited getTrailingObjectsImpl is not hidden.
    using ParentType::getTrailingObjectsImpl;

    // These two functions are helper functions for
    // TrailingObjects::getTrailingObjects. They recurse to the left --
    // the result for each type in the list of trailing types depends on
    // the result of calling the function on the type to the
    // left. However, the function for the type to the left is
    // implemented by a *subclass* of this class, so we invoke it via
    // the TopTrailingObj, which is, via the
    // curiously-recurring-template-pattern, the most-derived type in
    // this recursion, and thus, contains all the overloads.
    static const NextTy *getTrailingObjectsImpl(const BaseTy *Obj, TrailingObjectsBase::OverloadToken<NextTy>) {
        auto *Ptr = TopTrailingObj::getTrailingObjectsImpl(Obj, TrailingObjectsBase::OverloadToken<PrevTy>()) +
                    TopTrailingObj::callNumTrailingObjects(Obj, TrailingObjectsBase::OverloadToken<PrevTy>());

        if (requiresRealignment())
            return reinterpret_cast<const NextTy *>(alignAddr(Ptr, Align::Of<NextTy>()));
        else
            return reinterpret_cast<const NextTy *>(Ptr);
    }

    static NextTy *getTrailingObjectsImpl(BaseTy *Obj, TrailingObjectsBase::OverloadToken<NextTy>) {
        auto *Ptr = TopTrailingObj::getTrailingObjectsImpl(Obj, TrailingObjectsBase::OverloadToken<PrevTy>()) +
                    TopTrailingObj::callNumTrailingObjects(Obj, TrailingObjectsBase::OverloadToken<PrevTy>());

        if (requiresRealignment())
            return reinterpret_cast<NextTy *>(alignAddr(Ptr, Align::Of<NextTy>()));
        else
            return reinterpret_cast<NextTy *>(Ptr);
    }

    // Helper function for TrailingObjects::additionalSizeToAlloc: this
    // function recurses to superclasses, each of which requires one
    // fewer size_t argument, and adds its own size.
    static constexpr size_t additionalSizeToAllocImpl(size_t SizeSoFar, size_t Count1,
                                                      typename ExtractSecondType<MoreTys, size_t>::type... MoreCounts) {
        return ParentType::additionalSizeToAllocImpl(
            (requiresRealignment() ? alignTo<alignof(NextTy)>(SizeSoFar) : SizeSoFar) + sizeof(NextTy) * Count1,
            MoreCounts...);
    }
};

// The base case of the TrailingObjectsImpl inheritance recursion,
// when there's no more trailing types.
template <int Align, typename BaseTy, typename TopTrailingObj, typename PrevTy>
class TrailingObjectsImpl<Align, BaseTy, TopTrailingObj, PrevTy> : public TrailingObjectsAligner<Align> {
protected:
    // This is a dummy method, only here so the "using" doesn't fail --
    // it will never be called, because this function recurses backwards
    // up the inheritance chain to subclasses.
    static void getTrailingObjectsImpl();

    static constexpr size_t additionalSizeToAllocImpl(size_t SizeSoFar) {
        return SizeSoFar;
    }

    template <bool CheckAlignment> static void verifyTrailingObjectsAlignment() {}
};

} // namespace internal

template <typename BaseTy, typename... TrailingTys>
class TrailingObjects
    : private internal::TrailingObjectsImpl<internal::AlignmentCalcHelper<TrailingTys...>::Alignment, BaseTy,
                                            TrailingObjects<BaseTy, TrailingTys...>, BaseTy, TrailingTys...> {
    template <int A, typename B, typename T, typename P, typename... M> friend class internal::TrailingObjectsImpl;

    template <typename... Tys> class Foo {};

    typedef internal::TrailingObjectsImpl<internal::AlignmentCalcHelper<TrailingTys...>::Alignment, BaseTy,
                                          TrailingObjects<BaseTy, TrailingTys...>, BaseTy, TrailingTys...>
        ParentType;
    using TrailingObjectsBase = internal::TrailingObjectsBase;

    using ParentType::getTrailingObjectsImpl;

    // This function contains only a static_assert BaseTy is final. The
    // static_assert must be in a function, and not at class-level
    // because BaseTy isn't complete at class instantiation time, but
    // will be by the time this function is instantiated.
    static void verifyTrailingObjectsAssertions() {
        static_assert(std::is_final<BaseTy>(), "BaseTy must be final.");
    }

    // These two methods are the base of the recursion for this method.
    static const BaseTy *getTrailingObjectsImpl(const BaseTy *Obj, TrailingObjectsBase::OverloadToken<BaseTy>) {
        return Obj;
    }

    static BaseTy *getTrailingObjectsImpl(BaseTy *Obj, TrailingObjectsBase::OverloadToken<BaseTy>) {
        return Obj;
    }

    // callNumTrailingObjects simply calls numTrailingObjects on the
    // provided Obj -- except when the type being queried is BaseTy
    // itself. There is always only one of the base object, so that case
    // is handled here. (An additional benefit of indirecting through
    // this function is that consumers only say "friend
    // TrailingObjects", and thus, only this class itself can call the
    // numTrailingObjects function.)
    static size_t callNumTrailingObjects(const BaseTy *Obj, TrailingObjectsBase::OverloadToken<BaseTy>) {
        return 1;
    }

    template <typename T>
    static size_t callNumTrailingObjects(const BaseTy *Obj, TrailingObjectsBase::OverloadToken<T>) {
        return Obj->numTrailingObjects(TrailingObjectsBase::OverloadToken<T>());
    }

public:
    // Make this (privately inherited) member public.
    using ParentType::OverloadToken;

    /// Returns a pointer to the trailing object array of the given type
    /// (which must be one of those specified in the class template). The
    /// array may have zero or more elements in it.
    template <typename T> const T *getTrailingObjects() const {
        verifyTrailingObjectsAssertions();
        // Forwards to an impl function with overloads, since member
        // function templates can't be specialized.
        return this->getTrailingObjectsImpl(static_cast<const BaseTy *>(this), TrailingObjectsBase::OverloadToken<T>());
    }

    /// Returns a pointer to the trailing object array of the given type
    /// (which must be one of those specified in the class template). The
    /// array may have zero or more elements in it.
    template <typename T> T *getTrailingObjects() {
        verifyTrailingObjectsAssertions();
        // Forwards to an impl function with overloads, since member
        // function templates can't be specialized.
        return this->getTrailingObjectsImpl(static_cast<BaseTy *>(this), TrailingObjectsBase::OverloadToken<T>());
    }

    /// Returns the size of the trailing data, if an object were
    /// allocated with the given counts (The counts are in the same order
    /// as the template arguments). This does not include the size of the
    /// base object.  The template arguments must be the same as those
    /// used in the class; they are supplied here redundantly only so
    /// that it's clear what the counts are counting in callers.
    template <typename... Tys>
    static constexpr std::enable_if_t<std::is_same<Foo<TrailingTys...>, Foo<Tys...>>::value, size_t>
    additionalSizeToAlloc(typename internal::ExtractSecondType<TrailingTys, size_t>::type... Counts) {
        return ParentType::additionalSizeToAllocImpl(0, Counts...);
    }

    /// Returns the total size of an object if it were allocated with the
    /// given trailing object counts. This is the same as
    /// additionalSizeToAlloc, except it *does* include the size of the base
    /// object.
    template <typename... Tys>
    static constexpr std::enable_if_t<std::is_same<Foo<TrailingTys...>, Foo<Tys...>>::value, size_t>
    totalSizeToAlloc(typename internal::ExtractSecondType<TrailingTys, size_t>::type... Counts) {
        return sizeof(BaseTy) + ParentType::additionalSizeToAllocImpl(0, Counts...);
    }

    // We do not include the FixedSizeStorage class.
};

} // namespace sorbet::core

#endif // SORBET_TRAILING_OBJECTS_H
