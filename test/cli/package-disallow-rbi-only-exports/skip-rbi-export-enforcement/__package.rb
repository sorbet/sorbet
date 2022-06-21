# typed: strict

class Project::SkipRBIOnlyEnforcement < PackageSpec
  export Project::SkipRBIOnlyEnforcement::A # even though constant is only defined in .rbi file, no error as this path is skipped
end


