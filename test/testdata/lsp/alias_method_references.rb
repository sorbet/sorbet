# typed: true

class A
  def foo; end
  #    ^ def: A#foo
  alias_method :bar1, :foo
  #                    ^ usage: A#foo

  def bar2
    foo
  # ^ usage: A#foo
  end
end

class B
  extend T::Sig
  sig { returns(Symbol) }
  def self.foo
  #         ^ def: B.foo
    alias_method :bar, :foo
    #                   ^ usage: B.foo
  end
end