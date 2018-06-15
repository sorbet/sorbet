#include "version.h"

using namespace std;

namespace sorbet {
// NOTE: bazel ignores build flags for this.
// this means that: we can't use c++14\c++11 features here
// and this code has no debugger support

const string Version::version = "";  // 0.01 alpha
const string Version::codename = ""; // We Try Furiously

const string Version::build_scm_status = BUILD_SCM_STATUS; // non-release builds use "redacted" here

string makeScmRevision() {
    const string stamp = BUILD_SCM_REVISION;
    if (stamp != "0") {
        return stamp;
    }
    return "dev";
}

const string Version::build_scm_revision = makeScmRevision();
chrono::system_clock::time_point makeBuildTime() {
    const long buildStampMsec = BUILD_TIMESTAMP;
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
const bool Version::isReleaseBuild = build_scm_revision != "dev";

} // namespace sorbet
