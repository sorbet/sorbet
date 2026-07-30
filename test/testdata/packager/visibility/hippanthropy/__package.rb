# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Hippanthropy < PackageSpec
  import Foo # error: Package `Foo` includes explicit visibility modifiers and cannot be imported from `Hippanthropy`
end
