# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class KeywordArg < PackageSpec
  strict_dependencies 'false'
  layer(layer: 'a') # error: Expected `String` but found `{layer: String("a")}` for argument `arg0`
end
