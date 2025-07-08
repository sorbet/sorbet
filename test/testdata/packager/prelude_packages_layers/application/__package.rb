# frozen_string_literal: true
# typed: strict

class Application < PackageSpec # error: Layering violation: cannot import `Prelude::First`
  layer "utility"
  strict_dependencies "dag"

  export Application::B
end
