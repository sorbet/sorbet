# typed: strict

class Project::Baz < PackageSpec
  autoloader_compatibility :invalid
  autoloader_compatibility 'something'
end

