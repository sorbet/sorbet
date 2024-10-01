# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Repeated < PackageSpec
  strict_dependencies 'false'
  strict_dependencies 'false' # error: Repeated declaration of `strict_dependencies`
end
