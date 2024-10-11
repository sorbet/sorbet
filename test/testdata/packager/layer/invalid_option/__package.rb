# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class InvalidOption < PackageSpec
  strict_dependencies 'false'
  layer 'c' # error: Argument to `layer` must be one of: `a` or `b`
end
