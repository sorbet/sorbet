# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Prelude < PackageSpec
  prelude_package

  visible_to Application::A # error: Prelude package `Prelude` may not include `visible_to` annotations
  visible_to Application::B # error: Prelude package `Prelude` may not include `visible_to` annotations

  export Prelude::A
end
