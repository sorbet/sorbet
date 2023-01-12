# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo < PackageSpec
  visible_to Bar
  visible_to Quux
           # ^^^^ error: Unable to resolve constant `Quux`
end
