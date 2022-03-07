# typed: strict

class Family < PackageSpec

  import Family::Bart
  test_import Util::Testing

  export Family::Simpsons
  export Test::Family::TestFamily

  # As `Family::Flanders` leaks out through the interface of
  # `Test::Family::TestFamily`, it doesn't end up in the
  # `.test.private.package.rbi` file.
  export_for_test Family::Flanders

  # This name is only used internally in `Test::Family::TestFamily`, and as a
  # result will remain in the `.test.private.package.rbi` file
  export_for_test Family::Krabappel

end
