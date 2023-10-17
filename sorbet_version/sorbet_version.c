#include "sorbet_version/sorbet_version.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
/**
 * This file is *MAGIC*
 * When we compile it from Bazel, we do magical substitutions to some variables defined in CAPS in this file.
 * See `sorbet_version/BUILD` for up-to-date list of substitutions.
 *
 *  This file takes them and packages them to a API that is more pleasant to work with.
 */

// Manual configuration
#define SORBET_VERSION "0.5"
#define SORBET_CODENAME ""

// Magic configuration
#if BUILD_RELEASE
const int sorbet_is_release_build = 1;
// all these defines should be provided externally
#else
const int sorbet_is_release_build = 0;
#define STABLE_BUILD_SCM_CLEAN "0"
#define STABLE_BUILD_SCM_COMMIT_COUNT 0
#define BUILD_TIMESTAMP 0
#define STABLE_BUILD_SCM_REVISION "master"
#endif

// Wraps input in double quotes. https://stackoverflow.com/a/6671729
#define Q(x) #x
#define QUOTED(x) Q(x)

const char sorbet_build_scm_clean[] = STABLE_BUILD_SCM_CLEAN;
const char sorbet_build_scm_revision[] = STABLE_BUILD_SCM_REVISION;
const int sorbet_build_scm_commit_count = STABLE_BUILD_SCM_COMMIT_COUNT;
const long sorbet_build_timestamp = BUILD_TIMESTAMP;

#ifdef DEBUG_SYMBOLS
#define STABLE_BUILD_DEBUG_SYMBOLS "true"
const int sorbet_is_with_debug_symbols = 1;
#else
#define STABLE_BUILD_DEBUG_SYMBOLS "false"
const int sorbet_is_with_debug_symbols = 0;
#endif

const char sorbet_version[] = "0.5"; // 0.01 alpha
const char sorbet_codename[] = "";   // We Try Furiously

const char sorbet_full_version_string[] = SORBET_VERSION "." QUOTED(STABLE_BUILD_SCM_COMMIT_COUNT)
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

const char *sorbet_getVersion() {
    return sorbet_version;
}
const char *sorbet_getCodename() {
    return sorbet_codename;
}
const char *sorbet_getBuildSCMRevision() {
    return sorbet_build_scm_revision;
}
const int sorbet_getBuildSCMCommitCount() {
    return sorbet_build_scm_commit_count;
}
const char *sorbet_getBuildSCMClean() {
    return sorbet_build_scm_clean;
}
const long sorbet_getBuildTimestamp() {
    return sorbet_build_timestamp;
}
const char *sorbet_getFullVersionString() {
    return sorbet_full_version_string;
}
const int sorbet_getIsReleaseBuild() {
    return sorbet_is_release_build;
}
const int sorbet_getIsWithDebugSymbols() {
    return sorbet_is_with_debug_symbols;
}
