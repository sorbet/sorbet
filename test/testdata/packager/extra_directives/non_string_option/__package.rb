# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class NonStringOption < PackageSpec
  strict_dependencies 'false'
  layer 'a'
  extra 1 # error: Expected `String` but found `Integer(1)` for argument `x`
end
