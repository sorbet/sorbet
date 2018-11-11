#ifndef SORBET_VERSION_H
#define SORBET_VERSION_H

#include <chrono>
#include <string>

namespace sorbet {
class Version {
public:
    static const std::string version;
    static const std::string codename;
    static const std::string build_scm_revision;
    static const int build_scm_commit_count;
    static const std::string build_scm_status;
    static const std::chrono::system_clock::time_point build_timestamp;
    static const std::string build_timestamp_string;
    static const std::string full_version_string;
    static const bool isReleaseBuild;
    static const bool withDebugSymbols;
};
} // namespace sorbet

#endif // SORBET_VERSION_H
