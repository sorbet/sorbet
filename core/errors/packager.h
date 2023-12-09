#ifndef SORBET_CORE_ERRORS_PACKAGER_H
#define SORBET_CORE_ERRORS_PACKAGER_H
#include "core/Error.h"

namespace sorbet::core::errors::Packager {
// Packaging errors are fatal (they prevent packaging from working at runtime!), and should not be silenceable.
// Thus, all of these errors fire on typed: false and above.
// constexpr ErrorClass MustBeTypedStrict{3701, StrictLevel::False};
constexpr ErrorClass InvalidPackageDefinition{3702, StrictLevel::False};
constexpr ErrorClass RedefinitionOfPackage{3703, StrictLevel::False};
constexpr ErrorClass PackageNotFound{3704, StrictLevel::False};
constexpr ErrorClass UnpackagedFile{3705, StrictLevel::False};
constexpr ErrorClass InvalidConfiguration{3706, StrictLevel::False};
constexpr ErrorClass MultiplePackagesInOneFile{3707, StrictLevel::False};
// 3708 MultipleExportMethodsCalls
constexpr ErrorClass NoSelfImport{3709, StrictLevel::False};
constexpr ErrorClass InvalidPackageExpression{3710, StrictLevel::False};
constexpr ErrorClass PackageFileMustBeStrict{3711, StrictLevel::False};
constexpr ErrorClass InvalidPackageName{3712, StrictLevel::False};
constexpr ErrorClass DefinitionPackageMismatch{3713, StrictLevel::False};
// constexpr ErrorClass ImportConflict{3714, StrictLevel::False};
// constexpr ErrorClass InvalidExportForTest{3715, StrictLevel::False};
constexpr ErrorClass ExportConflict{3716, StrictLevel::False};
constexpr ErrorClass UsedPackagePrivateName{3717, StrictLevel::False};
constexpr ErrorClass MissingImport{3718, StrictLevel::False};
// constexpr ErrorClass PackagedSymbolInUnpackagedContext{3719, StrictLevel::False};
constexpr ErrorClass UsedTestOnlyName{3720, StrictLevel::False};
constexpr ErrorClass InvalidExport{3721, StrictLevel::False};
// constexpr ErrorClass ExportingTypeAlias{3722, StrictLevel::False};
constexpr ErrorClass ImportNotVisible{3723, StrictLevel::False};
} // namespace sorbet::core::errors::Packager
#endif
