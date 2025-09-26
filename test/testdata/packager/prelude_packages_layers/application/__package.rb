# frozen_string_literal: true
# typed: strict

class Application < PackageSpec
  layer "utility"
  strict_dependencies "dag"

  import Prelude::First # error: Layering violation
  import Prelude::Second

  export Application::B
end
