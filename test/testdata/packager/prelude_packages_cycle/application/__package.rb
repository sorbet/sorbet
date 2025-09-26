# frozen_string_literal: true
# typed: strict

class Application < PackageSpec
  layer 'a'
  strict_dependencies 'dag'

  import Prelude::First  # error: must be `dag` or higher
  import Prelude::Second # error: must be `dag` or higher

  export Application::B
end
