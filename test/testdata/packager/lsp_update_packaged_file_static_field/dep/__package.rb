# frozen_string_literal: true
# typed: strict

class Dep < PackageSpec
  import Package

  export Dep::Exports::ExportedClass
end
