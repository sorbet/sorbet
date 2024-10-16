# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class Repeated < PackageSpec
  strict_dependencies 'false'
  strict_dependencies 'false' # error: Repeated declaration of `strict_dependencies`
end
