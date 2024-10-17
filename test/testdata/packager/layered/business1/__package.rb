# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class Business1 < PackageSpec
  strict_dependencies 'layered'
  layer 'business'

  import Service1 # error: `Business1` is at layer `business`, so it can not import package `Service1`, which is at layer `service`
  import Business2
end
