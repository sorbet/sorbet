# typed: strict

class Project::Baz < PackageSpec
  autoloader_compatibility :invalid
  autoloader_compatibility 'something'
  autoloader_compatibility :legacy
end

