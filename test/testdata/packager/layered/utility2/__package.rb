# frozen_string_literal: true
# typed: strict

class Utility2 < PackageSpec
  strict_dependencies 'false'
  layer 'utility'

  import Utility1
  import Service1
end
