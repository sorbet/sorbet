# typed: true

# Because this constant has a leading `::`, the packager allows us to
# define things on it despite it not belonging to the enclosing namespace
class ::UnpackagedTheSequel
  # despite being in a constant not governed by this package, we still
  # subject the internals to import restrictions:
  def test
    # this is defined in unpackaged code, so okay
    puts UnpackagedCode

    # this is defined in an unpackaged RBI, so okay
    puts Minitest

    # this is defined within the same package, so okay
    puts MyPackage::MyClass.new

    # this is defined within an imported package, so okay
    puts OtherPackageImported::ExportedClass.new

    # this is defined in a non-imported package
    puts OtherPackageNotImported::ExportedClass.new
    #    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `OtherPackageNotImported::ExportedClass` resolves but its package is not imported
  end
end
