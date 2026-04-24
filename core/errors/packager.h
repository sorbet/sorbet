#ifndef SORBET_CORE_ERRORS_PACKAGER_H
#define SORBET_CORE_ERRORS_PACKAGER_H
#include "core/Error.h"

namespace sorbet::core::errors::Packager {
// Packaging errors are fatal and should not be silenceable.
// Thus, all of these errors fire on typed: false and above.
// inline constexpr ErrorClass MustBeTypedStrict{3701, StrictLevel::False};
inline constexpr ErrorClass InvalidPackageDefinition{3702, StrictLevel::False};
// inline constexpr ErrorClass RedefinitionOfPackage{3703, StrictLevel::False};
inline constexpr ErrorClass PackageNotFound{3704, StrictLevel::False};
inline constexpr ErrorClass UnpackagedFile{3705, StrictLevel::False};
inline constexpr ErrorClass InvalidConfiguration{3706, StrictLevel::False};
inline constexpr ErrorClass MultiplePackagesInOneFile{3707, StrictLevel::False};
// 3708 MultipleExportMethodsCalls
inline constexpr ErrorClass NoSelfImport{3709, StrictLevel::False};
inline constexpr ErrorClass InvalidPackageExpression{3710, StrictLevel::False};
inline constexpr ErrorClass PackageFileMustBeStrict{3711, StrictLevel::False};
inline constexpr ErrorClass InvalidPackageName{3712, StrictLevel::False};
inline constexpr ErrorClass DefinitionPackageMismatch{3713, StrictLevel::False};
// inline constexpr ErrorClass ImportConflict{3714, StrictLevel::False};
// inline constexpr ErrorClass InvalidExportForTest{3715, StrictLevel::False};
inline constexpr ErrorClass ExportConflict{3716, StrictLevel::False};
inline constexpr ErrorClass UsedPackagePrivateName{3717, StrictLevel::False};
inline constexpr ErrorClass MissingImport{3718, StrictLevel::False};
// inline constexpr ErrorClass PackagedSymbolInUnpackagedContext{3719, StrictLevel::False};
inline constexpr ErrorClass UsedTestOnlyName{3720, StrictLevel::False};
inline constexpr ErrorClass InvalidExport{3721, StrictLevel::False};
// inline constexpr ErrorClass ExportingTypeAlias{3722, StrictLevel::False};
inline constexpr ErrorClass ImportNotVisible{3723, StrictLevel::False};
inline constexpr ErrorClass InvalidStrictDependencies{3724, StrictLevel::False};
inline constexpr ErrorClass InvalidLayer{3725, StrictLevel::False};
inline constexpr ErrorClass LayeringViolation{3726, StrictLevel::False};
inline constexpr ErrorClass StrictDependenciesViolation{3727, StrictLevel::False};
inline constexpr ErrorClass DuplicateDirective{3728, StrictLevel::False};
inline constexpr ErrorClass InvalidMinTypedLevel{3729, StrictLevel::False};
// inline constexpr ErrorClass NoPreludeVisibleTo{3730, StrictLevel::False};
inline constexpr ErrorClass PreludePackageImport{3731, StrictLevel::False};
// inline constexpr ErrorClass NoExplicitPreludeImport{3732, StrictLevel::False};
// inline constexpr ErrorClass PreludeLowestLayer{3733, StrictLevel::False};
inline constexpr ErrorClass IncorrectPackageRB{3734, StrictLevel::False};
inline constexpr ErrorClass TestImportMismatch{3735, StrictLevel::False};
inline constexpr ErrorClass TypedSigilTooLow{3736, StrictLevel::False};
} // namespace sorbet::core::errors::Packager
#endif
