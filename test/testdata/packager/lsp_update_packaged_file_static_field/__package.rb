# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Package < PackageSpec
  import Dep

  export Package::A
end
