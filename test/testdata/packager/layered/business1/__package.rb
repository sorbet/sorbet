# frozen_string_literal: true
# typed: strict

class Business1 < PackageSpec
  strict_dependencies 'layered'
  layer 'business'

  import Service1 # error: Layering violation: cannot import `Service1` (in layer `service`) from `Business1` (in layer `business`)
  import Business2
end
