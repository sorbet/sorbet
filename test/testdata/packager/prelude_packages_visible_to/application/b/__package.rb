# frozen_string_literal: true
# typed: strict

class Application::B < PackageSpec
  import Prelude # error: cannot be imported

  export Application::B::Foo
end
