# typed: strict

module Opus
  module Foo
    module Foo::D # ambiguous
    end

    module Bar::C # unambiguous -- we do not currently check across package boundaries
    end
  end
end
