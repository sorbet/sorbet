# frozen_string_literal: true
# typed: strict

class Application < PackageSpec
  layer "utility"
  strict_dependencies "dag"

  export Application::B
end
