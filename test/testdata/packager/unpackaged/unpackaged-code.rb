# frozen_string_literal: true
# typed: true
# enable-packager: true

# Despite this being in a packaged context, this constant is
# unpackaged and therefore can be freely used by packaged code.
module UnpackagedCode
  # Referencing packaged constants in unpackaged code is not allowed,
  # though
  MyPackage::MyClass
# ^^^^^^^^^^^^^^^^^^ error: `MyPackage::MyClass` is defined in a package and cannot
end
