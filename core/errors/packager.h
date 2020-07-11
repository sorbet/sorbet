#ifndef SORBET_CORE_ERRORS_PACKAGER_H
#define SORBET_CORE_ERRORS_PACKAGER_H
#include "core/Error.h"

namespace sorbet::core::errors::Packager {
constexpr ErrorClass MustBeTypedStrict{8001, StrictLevel::False};
constexpr ErrorClass InvalidPackageDefinition{8002, StrictLevel::False};
constexpr ErrorClass RedefinitionOfPackage{8003, StrictLevel::False};
constexpr ErrorClass PackageNotFound{8004, StrictLevel::False};
constexpr ErrorClass UnpackagedFile{8005, StrictLevel::False};
constexpr ErrorClass InvalidImportOrExport{8006, StrictLevel::False};
constexpr ErrorClass MultiplePackagesInOneFile{8007, StrictLevel::False};
constexpr ErrorClass MultipleExportMethodsCalls{8008, StrictLevel::False};
constexpr ErrorClass NoSelfImport{8009, StrictLevel::False};
constexpr ErrorClass InvalidPackageExpression{8010, StrictLevel::False};
constexpr ErrorClass PackageFileMustBeStrict{8011, StrictLevel::False};
} // namespace sorbet::core::errors::Packager
#endif
