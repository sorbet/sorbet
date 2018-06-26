#include "version/version.h"

using namespace std;

namespace sorbet {
// NOTE: bazel ignores build flags for this.
// this means that: we can't use c++14\c++11 features here
// and this code has no debugger support

#if BUILD_RELEASE
const char *const build_scm_clean = STABLE_BUILD_SCM_CLEAN;
const char *const build_scm_revision = STABLE_BUILD_SCM_REVISION;
const long build_timestamp = BUILD_TIMESTAMP;
constexpr bool is_release_build = true;
#else
const char *const build_scm_clean = "1";
const char *const build_scm_revision = "master";
const long build_timestamp = 0;
constexpr bool is_release_build = false;
#endif

const string Version::version = "";  // 0.01 alpha
const string Version::codename = ""; // We Try Furiously

const bool Version::isReleaseBuild = is_release_build;

string makeScmStatus() {
    return strncmp(build_scm_clean, "0", 1) == 0 ? "-dirty" : "";
}
const string Version::build_scm_status = makeScmStatus();

string makeScmRevision() {
    return build_scm_revision;
}
const string Version::build_scm_revision = makeScmRevision();

chrono::system_clock::time_point makeBuildTime() {
    const long buildStampMsec = build_timestamp;
    chrono::system_clock::time_point res((chrono::milliseconds(buildStampMsec)));
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
const string Version::build_timestamp_string = makeBuildTimeString(); // non-release build have 1970-01-01 00:00:00 GMT

} // namespace sorbet
