# frozen_string_literal: true
# typed: strict

class Business2 < PackageSpec
  strict_dependencies 'layered'
  layer 'business'

  import Utility1
  test_import Service1
end
