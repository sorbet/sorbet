# typed: strict

class Project::A < PackageSpec
  visible_to Project::B
end
