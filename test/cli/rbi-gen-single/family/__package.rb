# typed: strict

class Family < PackageSpec

  import Family::Bart
  test_import Util::Testing

  export Family::Simpsons
  export Test::Family::TestFamily

  # This illustrates a problem: the `Family::Flanders` symbol ends up in the
  # .test.package.rbi file for this package, which means that it's available to
  # all packages that `test_import` the `Family` package. The right solution
  # here is to output a third rbi that represents the shared but private
  # interface between a package and its test package.
  export_for_test Family::Flanders

end
