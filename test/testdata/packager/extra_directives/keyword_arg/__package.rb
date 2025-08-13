# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a, b

class KeywordArg < PackageSpec
  strict_dependencies 'false'
  layer 'a'
  extra(arg: 'a')
  #     ^^^^^^^^ error: Expected `String` but found `{arg: String("a")}` for argument `x`
end
