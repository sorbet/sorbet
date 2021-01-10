# frozen_string_literal: true
# typed: strict

class Project::Foo < PackageSpec
  export Foo
  export_methods FooMethods
end
