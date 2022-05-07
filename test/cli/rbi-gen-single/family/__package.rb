# typed: strict

class Family < PackageSpec

  import External
  import Family::Bart
  test_import Util::Testing

  export Family::Belcher
  export Family::Simpsons
  export Test::Family::TestFamily

  export_for_test Family::Flanders

  export_for_test Family::Krabappel

end
