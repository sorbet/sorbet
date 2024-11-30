# frozen_string_literal: true
# typed: strict

class Project::Foo < PackageSpec # error: Package `Project::Foo` is missing imports
end
