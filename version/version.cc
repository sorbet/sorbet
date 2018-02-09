#include "version.h"

namespace ruby_typer {
// NOTE: bazel ignores build flags for this.
// this means that: we can't use c++14\c++11 features here
// and this code has no debugger support

const std::string Version::version = "";  // 0.01 alpha
const std::string Version::codename = ""; // We Try Furiously
const std::string Version::build_scm_revision = BUILD_SCM_REVISION;
const std::string Version::build_scm_status = BUILD_SCM_STATUS;

std::chrono::system_clock::time_point makeBuildTime() {
    const long buildStampMsec = BUILD_TIMESTAMP;
    std::chrono::system_clock::time_point res((std::chrono::milliseconds(buildStampMsec)));
    return res;
}

const std::chrono::system_clock::time_point Version::build_timestamp = makeBuildTime();

std::string makeBuildTimeString() {
    std::time_t timet = std::chrono::system_clock::to_time_t(Version::build_timestamp);
    std::tm *gmtTime = std::gmtime(&timet);
    char buffer[512];
    int written = std::strftime(buffer, sizeof(buffer), "%F %T GMT", gmtTime);
    buffer[written] = '\0';
    return buffer;
}

const std::string Version::build_timestamp_string = makeBuildTimeString();

} // namespace ruby_typer
