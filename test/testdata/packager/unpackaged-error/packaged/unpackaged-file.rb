# typed: true
# packaged: false
#
# Explicitly marking this file as not packaged to show how to opt-out of being associated with the package in the path.

class ::UnpackagedTheSequel
  def test

    puts MyPackage::MyClass.new
    puts OtherPackageImported::ExportedClass.new
    puts OtherPackageNotImported::ExportedClass.new
  end
end
