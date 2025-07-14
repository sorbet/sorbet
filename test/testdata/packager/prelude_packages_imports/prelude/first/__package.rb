# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Prelude::First < PackageSpec
  prelude_package

  export Prelude::First::A
end
