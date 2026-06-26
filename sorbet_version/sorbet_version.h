#ifndef SORBET_VERSION_H
#define SORBET_VERSION_H

#if !defined(NDEBUG) || defined(FORCE_DEBUG)
#define DEBUG_MODE
#else
#undef DEBUG_MODE
#endif

// In release builds, this generated header defines macros with real values from
// Bazel's workspace status file. In non-release builds, it is empty and the
// #ifndef defaults below apply.
#include "sorbet_version/sorbet_version_defines.h"

// Wraps input in double quotes. https://stackoverflow.com/a/6671729
#define Q_(x) #x
#define QUOTED_(x) Q_(x)

#define SORBET_VERSION "0.6"

namespace sorbet {

#ifdef DEBUG_MODE
inline constexpr bool debug_mode = true;
#else
inline constexpr bool debug_mode = false;
#endif

#ifdef TRACK_UNTYPED_BLAME_MODE
inline constexpr bool track_untyped_blame_mode = true;
#else
inline constexpr bool track_untyped_blame_mode = false;
#endif

#if !defined(EMSCRIPTEN)
inline constexpr bool emscripten_build = false;
#else
inline constexpr bool emscripten_build = true;
#endif

#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
inline constexpr bool fuzz_mode = false;
#else
inline constexpr bool fuzz_mode = true;
#endif

#ifndef BUILD_RELEASE
#define BUILD_RELEASE 0
#endif
inline constexpr bool is_release_build = BUILD_RELEASE;

#ifndef STABLE_BUILD_SCM_CLEAN
#define STABLE_BUILD_SCM_CLEAN "0"
#endif
inline constexpr char build_scm_clean[] = STABLE_BUILD_SCM_CLEAN;

#ifndef STABLE_BUILD_SCM_REVISION
#define STABLE_BUILD_SCM_REVISION "master"
#endif
inline constexpr char build_scm_revision[] = STABLE_BUILD_SCM_REVISION;

#ifndef STABLE_BUILD_SCM_COMMIT_COUNT
#define STABLE_BUILD_SCM_COMMIT_COUNT 0
#endif
inline constexpr int build_scm_commit_count = STABLE_BUILD_SCM_COMMIT_COUNT;

#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP 0
#endif
inline constexpr long build_timestamp = BUILD_TIMESTAMP;

#ifdef DEBUG_SYMBOLS
#define STABLE_BUILD_DEBUG_SYMBOLS "true"
inline constexpr bool is_with_debug_symbols = true;
#else
#define STABLE_BUILD_DEBUG_SYMBOLS "false"
inline constexpr bool is_with_debug_symbols = false;
#endif

inline constexpr char full_version_string[] = SORBET_VERSION "." QUOTED_(STABLE_BUILD_SCM_COMMIT_COUNT)
#if BUILD_RELEASE
    " git " STABLE_BUILD_SCM_REVISION
#else
    " (non-release)"
#endif
    " debug_symbols=" STABLE_BUILD_DEBUG_SYMBOLS " clean=" STABLE_BUILD_SCM_CLEAN
#ifdef DEBUG_MODE
    " debug_mode=true"
#endif
#ifdef TRACK_UNTYPED_BLAME_MODE
    " untyped_blame=true"
#endif
    ;

} // namespace sorbet

#undef Q_
#undef QUOTED_

#endif // SORBET_VERSION_H
