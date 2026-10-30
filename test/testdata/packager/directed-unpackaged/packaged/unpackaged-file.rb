# typed: true

# Because this constant has a leading `::`, the packager allows us to
# define things on it despite it not belonging to the enclosing namespace
class ::UnpackagedTheSequel # error: requires this package to be marked `prelude!`
  # despite being in a constant not governed by this package, we still
  # subject the internals to import restrictions:
  def test
    # unpackaged Ruby definitions are isolated
    puts UnpackagedCode # error: Unable to resolve constant `UnpackagedCode`

    # unpackaged RBIs are isolated too (only Sorbet payload files are exempt)
    puts Minitest # error: Unable to resolve constant `Minitest`

    # this is defined within the same package, so okay
    puts MyPackage::MyClass.new

    # this is defined within an imported package, so okay
    puts OtherPackageImported::ExportedClass.new

    # this is defined in a non-imported package
    puts OtherPackageNotImported::ExportedClass.new
    #    ^^^^^^^^^^^^^^^^^^^^^^^ error: `OtherPackageNotImported` is not imported
  end
end
