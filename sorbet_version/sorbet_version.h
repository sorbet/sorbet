#ifndef SORBET_VERSION_H
#define SORBET_VERSION_H

#if !defined(NDEBUG) || defined(FORCE_DEBUG)
#define DEBUG_MODE
#else
#undef DEBUG_MODE
#endif

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

extern const char version[];
extern const char codename[];
extern const char build_scm_revision[];
extern const int build_scm_commit_count;
extern const char build_scm_clean[];
extern const long build_timestamp;
extern const char full_version_string[];
extern const int is_release_build;
extern const int is_with_debug_symbols;

} // namespace sorbet

#endif // SORBET_VERSION_H
