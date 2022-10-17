# typed: strict
# frozen_string_literal: true

class RootPackage < PackageSpec
  autoloader_compatibility 'legacy'

  export RootPackage::Yabba
end
