# typed: strict

class Project::Foo < PackageSpec
  restrict_to_service Project::Foo
  restrict_to_service Project::Bar
end
