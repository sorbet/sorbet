# typed: strict

module Opus
  module Bar
    module B; end

    module Foo::C; end # unambiguous
  end
end
