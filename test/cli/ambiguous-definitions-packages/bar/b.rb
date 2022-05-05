# typed: strict

module Opus
  module Bar
    module B; end

    module Foo::C; end # ambiguous, despite not importing the foo package
  end
end
