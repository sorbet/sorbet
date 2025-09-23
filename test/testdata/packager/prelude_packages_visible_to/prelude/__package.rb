# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Prelude < PackageSpec
  prelude_package

  visible_to Application::A

  export Prelude::A
end
