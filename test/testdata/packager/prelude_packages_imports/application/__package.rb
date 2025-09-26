# frozen_string_literal: true
# typed: strict

class Application < PackageSpec
  import Prelude::First
  test_import Prelude::Second
end
