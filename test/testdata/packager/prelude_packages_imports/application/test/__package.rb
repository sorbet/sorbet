# frozen_string_literal: true
# typed: strict

class Test::Application < PackageSpec
  test!

  import Application
  import Prelude::Second
end
