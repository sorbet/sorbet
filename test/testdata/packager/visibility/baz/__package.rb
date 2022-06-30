# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Baz < PackageSpec
  import Foo # error: Package `Foo` includes explicit visibility modifiers and does not allow imports from `Baz`
end
