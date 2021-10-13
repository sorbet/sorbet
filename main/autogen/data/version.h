#ifndef AUTOGEN_VERSION_H
#define AUTOGEN_VERSION_H

namespace sorbet::autogen {

class AutogenVersion {
public:
    constexpr static int MIN_VERSION = 2;
    constexpr static int MAX_VERSION = 4;

    // Version history:
    // 3 - Add type alias information in dependency db output
    // 4 - Include .rbi reference information in dependency db
    constexpr static int VERSION_INCLUDE_RBI = 4;
};

} // namespace sorbet::autogen

#endif // AUTOGEN_VERSION_H
