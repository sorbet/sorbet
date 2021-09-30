# typed: strict

module Opus::Foo
  class FooClass; end

  class Private::ImplDetail
    extend T::Sig
    sig {void}
    def self.stub_stuff!; end
  end
end
