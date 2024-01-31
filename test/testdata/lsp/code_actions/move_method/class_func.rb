# typed: strict
# selective-apply-code-action: refactor.extract

module Foo
  extend T::Sig

  sig {returns(String)}
  def self.greeting
         # | apply-code-action: [A] Move method to a new module
    'Hello'
  end

  sig do
    params(x: String)
    .returns(String)
  end
  def name(x)
    "#{Foo.greeting} #{x}"
  end
end

module A
  class B
    extend T::Sig

    sig {void}
    def bar
      m = Foo
      Foo.greeting

      m.greeting
      print((Foo if true).greeting)
      print((if T.unsafe(true); Foo; else Foo; end).greeting)
      print((if T.unsafe(true); Foo; else Integer; end).greeting) # error: does not exist
    end

    sig {params(m: T.class_of(Foo)).void}
    def example3(m)
      m.greeting
    end
  end
end
Foo.greeting
