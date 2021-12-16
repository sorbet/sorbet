# typed: strict

module Opus
  module Foo
    module Foo::D # ambiguous
    end

    module Bar::C # ambiguous
    end
  end
end
