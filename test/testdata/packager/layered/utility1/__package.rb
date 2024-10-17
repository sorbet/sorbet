# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class Utility1 < PackageSpec
  strict_dependencies 'layered'
  layer 'utility'
end
