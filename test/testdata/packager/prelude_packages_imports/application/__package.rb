# frozen_string_literal: true
# typed: strict

class Application < PackageSpec
  import Prelude::First # error: Prelude package `Prelude::First` may not be explicitly imported
  test_import Prelude::Second # error: Prelude package `Prelude::Second` may not be explicitly imported
end
