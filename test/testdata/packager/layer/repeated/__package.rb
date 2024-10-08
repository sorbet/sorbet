# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class Repeated < PackageSpec
  strict_dependencies 'false'
  layer 'a'
  layer 'a' # error: Repeated declaration of `layer`
end
