#ifndef AUTOGEN_VERSION_H
#define AUTOGEN_VERSION_H

namespace sorbet::autogen {

class AutogenVersion {
public:
    constexpr static int MIN_VERSION = 6;
    constexpr static int MAX_VERSION = 7;

    // Version history:
    // 3 - Add type alias information in dependency db output
    // 4 - Include .rbi reference information in dependency db
    // 5 - Pack information more tightly in various places
    // 6 - deduplicated nesting vectors, better `resolved` on refs, "lite" db, better symbol placement
    // 7 - no array headers for defs/refs, more information in "lite" db
    constexpr static int VERSION_INCLUDE_RBI = 4;
};

} // namespace sorbet::autogen

#endif // AUTOGEN_VERSION_H
