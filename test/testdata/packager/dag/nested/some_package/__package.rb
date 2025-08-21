# frozen_string_literal: true
# typed: strict

class Nested::SomePackage < PackageSpec
  strict_dependencies 'dag'
  layer 'utility'
end
