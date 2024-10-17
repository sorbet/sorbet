# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class Utility2 < PackageSpec
  strict_dependencies 'false'
  layer 'utility'

  import Utility1
  import Service1
end
