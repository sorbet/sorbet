#ifndef SORBET_CORE_ERRORS_PACKAGER_H
#define SORBET_CORE_ERRORS_PACKAGER_H
#include "core/Error.h"

namespace sorbet::core::errors::Packager {
constexpr ErrorClass MustBeTypedStrict{8001, StrictLevel::False};
constexpr ErrorClass InvalidPackageDefinition{8002, StrictLevel::False};
constexpr ErrorClass InvalidImportOrExport{8003, StrictLevel::False};
constexpr ErrorClass DuplicatePackageName{8004, StrictLevel::False};
constexpr ErrorClass PackageNotFound{8005, StrictLevel::False};
} // namespace sorbet::core::errors::Packager
#endif
