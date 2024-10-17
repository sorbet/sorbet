# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class Business2 < PackageSpec
  strict_dependencies 'layered'
  layer 'business'

  import Utility1
  test_import Service1
end
