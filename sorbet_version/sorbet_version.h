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
constexpr bool debug_mode = true;
#else
constexpr bool debug_mode = false;
#endif

#ifdef TRACK_UNTYPED_BLAME_MODE
constexpr bool track_untyped_blame_mode = true;
#else
constexpr bool track_untyped_blame_mode = false;
#endif

#if !defined(EMSCRIPTEN)
constexpr bool emscripten_build = false;
#else
constexpr bool emscripten_build = true;
#endif

#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
constexpr bool fuzz_mode = false;
#else
constexpr bool fuzz_mode = true;
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

// Linking against a function symbol suffers fewer pitfalls vs linking against a data symbol, especially when it's
// possible the symbol could be multiply defined (it's easier to make a function symbol weak than it is to make a
// constant global variable symbol weak: https://stackoverflow.com/questions/36087831).
//
// But using a data symbol directly can sometimes give better optimized code when there are no linking
// concerns (e.g., we're only statically linking and all source code is available).
//
// Thus we provide both options so people can choose based on their needs. tl;dr if you're writing code inside Sorbet
// itself, prefer the global variables above.
const char *sorbet_getVersion();
const char *sorbet_getCodename();
const char *sorbet_getBuildSCMRevision();
const int sorbet_getBuildSCMCommitCount();
const char *sorbet_getBuildSCMClean();
const long sorbet_getBuildTimestamp();
const char *sorbet_getFullVersionString();
const int sorbet_getIsReleaseBuild();
const int sorbet_getIsWithDebugSymbols();

#ifdef __cplusplus
}
#endif

#endif // SORBET_VERSION_H
