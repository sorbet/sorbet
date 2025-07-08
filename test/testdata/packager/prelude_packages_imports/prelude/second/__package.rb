# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Prelude::Second < PackageSpec
  prelude_package

  export Prelude::Second::B
end
