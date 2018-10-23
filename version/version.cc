#include "version/version.h"

/**
 * This file is *MAGIC*
 * When we compile if from Bazel, we do magical substitutions to some variables defined in CAPS in this file.
 * See `version/Build` for up-to-date list of substitutions.
 *
 *  This file takes them and packages them to a API that is more pleasant to work with.
 */
using namespace std;

namespace sorbet {
// NOTE: bazel ignores build flags for this.
// this means that: we can't use c++14\c++11 features here
// and this code has no debugger support

#if BUILD_RELEASE
constexpr string_view build_scm_clean = STABLE_BUILD_SCM_CLEAN;
constexpr string_view build_scm_revision = STABLE_BUILD_SCM_REVISION;
constexpr string_view build_scm_commit_count = STABLE_BUILD_SCM_COMMIT_COUNT;
constexpr long build_timestamp = BUILD_TIMESTAMP;
constexpr bool is_release_build = true;
#else
constexpr string_view build_scm_clean = "1";
constexpr string_view build_scm_revision = "master";
constexpr string_view build_scm_commit_count = "0";
constexpr long build_timestamp = 0;
constexpr bool is_release_build = false;
#endif

#if DEBUG_SYMBOLS
const bool Version::withDebugSymbols = true;
#else
const bool Version::withDebugSymbols = false;
#endif

const string Version::version = "";  // 0.01 alpha
const string Version::codename = ""; // We Try Furiously

const bool Version::isReleaseBuild = is_release_build;

string makeScmStatus() {
    return build_scm_clean == "0"sv ? "-dirty" : "";
}
const string Version::build_scm_status = makeScmStatus();

string makeScmRevision() {
    return string(build_scm_revision);
}
const string Version::build_scm_revision = makeScmRevision();

string makeScmCommitCount() {
    return string(build_scm_commit_count);
}
const string Version::build_scm_commit_count = makeScmCommitCount();

chrono::system_clock::time_point makeBuildTime() {
    const long buildStampSec = build_timestamp;
    chrono::system_clock::time_point res((chrono::seconds(buildStampSec)));
    return res;
}
const chrono::system_clock::time_point Version::build_timestamp = makeBuildTime();

string makeBuildTimeString() {
    time_t timet = chrono::system_clock::to_time_t(Version::build_timestamp);
    tm *gmtTime = gmtime(&timet);
    char buffer[512];
    int written = strftime(buffer, sizeof(buffer), "%F %T GMT", gmtTime);
    buffer[written] = '\0';
    return buffer;
}
const string Version::build_timestamp_string = makeBuildTimeString(); // non-release builds have 1970-01-01 00:00:00 GMT

const string Version::full_version_string =
    Version::version + Version::codename + "rev " + Version::build_scm_commit_count +
    (Version::isReleaseBuild ? "" : " (non-release)") + " git " + Version::build_scm_revision +
    Version::build_scm_status + " built on " + Version::build_timestamp_string +
    (Version::withDebugSymbols ? " with debug symbols" : "");
} // namespace sorbet
