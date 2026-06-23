#ifndef SORBET_VERSION_H
#define SORBET_VERSION_H

// We want this header to be both a C++ and a C header, so that it can be depended on by very
// low-level things
#ifdef __cplusplus
extern "C" {
#endif

#if !defined(NDEBUG) || defined(FORCE_DEBUG)
#define DEBUG_MODE
#else
#undef DEBUG_MODE
#endif

#ifdef __cplusplus
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

} // namespace sorbet
#endif
// ^^^ __cplusplus

extern const char sorbet_version[];
extern const char sorbet_codename[];
extern const char sorbet_build_scm_revision[];
extern const int sorbet_build_scm_commit_count;
extern const char sorbet_build_scm_clean[];
extern const long sorbet_build_timestamp;
extern const char sorbet_full_version_string[];
extern const int sorbet_is_release_build;
extern const int sorbet_is_with_debug_symbols;

#ifdef __cplusplus
}
#endif

#endif // SORBET_VERSION_H
