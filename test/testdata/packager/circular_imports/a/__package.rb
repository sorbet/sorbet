# frozen_string_literal: true
# typed: strict
# enable-packager: true

class A < PackageSpec
  import B

  export A::AClass
end
