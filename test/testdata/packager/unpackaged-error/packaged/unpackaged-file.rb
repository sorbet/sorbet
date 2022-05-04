# typed: true
# packaged: false
#
# Explicitly marking this file as not packaged to show how to opt-out of being associated with the package in the path.

class ::UnpackagedTheSequel
  def test

    # This reference should be an error.
    puts MyPackage::MyClass.new
  end
end
