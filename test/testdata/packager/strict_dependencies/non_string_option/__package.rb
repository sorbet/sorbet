# frozen_string_literal: true
# typed: strict
# enable-packager: true
# packager-layers: a

class NonStringOption < PackageSpec
  strict_dependencies false # error: Argument to `strict_dependencies` must be one of: `'false'`, `'layered'`, `'layered_dag'`, or `'dag'`
#                     ^^^^^ error: Expected `String` but found `FalseClass` for argument `arg0`
end
