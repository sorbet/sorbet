# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class KeywordArg < PackageSpec
  strict_dependencies(level: 'false') # error: Expected `String` but found `{level: String("false")}` for argument `arg0`
end
