#ifndef AUTOGEN_VERSION_H
#define AUTOGEN_VERSION_H

namespace sorbet::autogen {

class AutogenVersion {
public:
    constexpr static int MIN_VERSION = 5;
    constexpr static int MAX_VERSION = 6;

    // Version history:
    // 3 - Add type alias information in dependency db output
    // 4 - Include .rbi reference information in dependency db
    // 5 - Pack information more tightly in various places
    constexpr static int VERSION_INCLUDE_RBI = 4;
};

} // namespace sorbet::autogen

#endif // AUTOGEN_VERSION_H
