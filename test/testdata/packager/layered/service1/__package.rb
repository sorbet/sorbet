# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: utility,power,business,api,service

class Service1 < PackageSpec
  strict_dependencies 'layered'
  layer 'service'

  import Business1
  import Utility2 # error: All of this package's dependecies must be `layered` or higher
end
