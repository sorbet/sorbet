# typed: strict

class Project::Root < PackageSpec
  export Project::Root::A  # no error since constant is defined in .rb file
  export Project::Root::B # error since constant is only defined in .rbi file
end

