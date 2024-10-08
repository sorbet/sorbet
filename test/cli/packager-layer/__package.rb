# typed: strict

class Project::Root < PackageSpec
  strict_dependencies 'false'
  layer 'fake'
end
