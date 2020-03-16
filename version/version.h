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

extern const char *sorbet_version;
extern const char *sorbet_codename;
extern const char *sorbet_build_scm_revision;
extern const int sorbet_build_scm_commit_count;
extern const char *sorbet_build_scm_status;
extern const long sorbet_build_timestamp;
extern const char *sorbet_full_version_string;
extern const int sorbet_isReleaseBuild;
extern const int sorbet_isWithDebugSymbols;

#ifdef __cplusplus
}
#endif

#endif // SORBET_VERSION_H
