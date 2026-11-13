# frozen_string_literal: true
# typed: strict

class Application::B < PackageSpec
  import Prelude # error: cannot be imported
end
