# frozen_string_literal: true
# typed: strict

class B < PackageSpec
  strict_dependencies 'layered'
  layer 'business'

  import A
end
