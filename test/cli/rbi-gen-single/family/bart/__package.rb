# typed: strict

class Family::Bart < PackageSpec

  import Family
  import Family::Bart::Slingshot
  import Util
  test_import Util::Testing

  export Family::Bart::Character
  export Test::Family::Bart::BartTest

end
