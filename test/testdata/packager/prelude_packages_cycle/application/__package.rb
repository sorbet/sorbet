# frozen_string_literal: true
# typed: strict

class Application < PackageSpec
  # error: must be `dag` or higher
  # error: must be `dag` or higher
  layer 'a'
  strict_dependencies 'dag'

  export Application::B
end
