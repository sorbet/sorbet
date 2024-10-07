# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class NonStringOption < PackageSpec
  strict_dependencies 'false'
  layer 1 # error: Argument to `layer` must be one of: a, or b
#       ^ error: Expected `String` but found `Integer(1)` for argument `arg0`
end
