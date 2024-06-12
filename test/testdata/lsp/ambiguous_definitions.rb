# typed: strict

# no-op comment
module Opus
  module Foo
    module Foo::Bar # error: Definition of `Bar` is ambiguous
    end
  end
end
