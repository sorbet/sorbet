# frozen_string_literal: true
# typed: true
# enable-packager: true
# enable-package-directed: true

# stratum: 0

# Despite this being in a packaged context, this constant is
# unpackaged and therefore can be freely used by packaged code.
module UnpackagedCode
  MyPackage::MyClass
# ^^^^^^^^^ error: Unable to resolve constant `MyPackage`
end
