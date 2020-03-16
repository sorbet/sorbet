#include "version/version.h"
#include <cstdlib>
#include <cstring>
#include <ctime>
/**
 * This file is *MAGIC*
 * When we compile if from Bazel, we do magical substitutions to some variables defined in CAPS in this file.
 * See `version/Build` for up-to-date list of substitutions.
 *
 *  This file takes them and packages them to a API that is more pleasant to work with.
 */

extern "C" {

// NOTE: bazel ignores build flags for this.
// this means that: we can't use c++14\c++11 features here
// and this code has no debugger support

// Manual configuration
#define SORBET_VERSION "0.5"
#define SORBET_CODENAME ""

// Magic configuration
#if BUILD_RELEASE
const int sorbet_isReleaseBuild = 1;
// all these defines should be provided externally
#else
const int sorbet_isReleaseBuild = 0;
#define STABLE_BUILD_SCM_CLEAN "0"
#define STABLE_BUILD_SCM_COMMIT_COUNT "0"
#define BUILD_TIMESTAMP 0
#define STABLE_BUILD_SCM_REVISION "master"
#endif

const char *sorbet_build_scm_clean = STABLE_BUILD_SCM_CLEAN;
const char *sorbet_build_scm_revision = STABLE_BUILD_SCM_REVISION;
const int sorbet_build_scm_commit_count = atoi(STABLE_BUILD_SCM_COMMIT_COUNT);
const long sorbet_build_timestamp = BUILD_TIMESTAMP;

#ifdef DEBUG_SYMBOLS
#define STABLE_BUILD_DEBUG_SYMBOLS "true"
const int sorbet_isWithDebugSymbols = 1;
#else
#define STABLE_BUILD_DEBUG_SYMBOLS "false"
const int sorbet_isWithDebugSymbols = 0;
#endif

const char *sorbet_version = "0.5"; // 0.01 alpha
const char *sorbet_codename = "";   // We Try Furiously

static const char *append_build_in_release(const char *arg) {
#if BUILD_RELEASE
    time_t timet = sorbet_build_timestamp;
    tm *gmtTime = gmtime(&timet);
    char *buffer = (char *)malloc(strlen(arg) + 100);
    memcpy(buffer, arg, strlen(arg));
    int written = strftime(buffer + strlen(arg), 100, "build on %F %T GMT",
                           gmtTime); // non-release builds have 1970-01-01 00:00:00 GMT
    buffer[written + strlen(arg)] = '\0';
    return buffer;
#else
    return arg;
#endif
}

const char *sorbet_full_version_string =
    append_build_in_release(SORBET_VERSION "." STABLE_BUILD_SCM_COMMIT_COUNT
#if BUILD_RELEASE
                                           " git " STABLE_BUILD_SCM_REVISION
#else
                                           " (non-release)"
#endif
                                           "debug_symbols=" STABLE_BUILD_DEBUG_SYMBOLS " clean=" STABLE_BUILD_SCM_CLEAN
#ifdef DEBUG_MODE
                                           " debug_mode=true"
#endif
    );
}
